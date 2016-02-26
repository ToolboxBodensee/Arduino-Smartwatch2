// You can use any (4 or) 5 pins
#define sclk 2
#define mosi 3
#define dc   5
#define cs   6
#define rst  9

#define BLUEFRUIT_SPI_CS               8
#define BLUEFRUIT_SPI_IRQ              7
#define BLUEFRUIT_SPI_RST              4

// Color definitions
#define	BLACK           0x0000
#define	BLUE            0x001F
#define	RED             0xF800
#define	GREEN           0x07E0
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0
#define WHITE           0xFFFF

#define NOTIF_COLOR BLUE
#define CLOCK_COLOR WHITE

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include <Adafruit_BluefruitLE_SPI.h>
#include <SPI.h>
#include <SoftwareSerial.h>

#include <avr/delay.h>

#define BUFSIZE                        32   // Size of the read buffer for incoming data
#define VERBOSE_MODE                   false  // If set to 'true' enables debug output
#define FACTORYRESET_ENABLE         1
#define MINIMUM_FIRMWARE_VERSION    "0.6.6"
#define MODE_LED_BEHAVIOUR          "DISABLE"

#define SETLED (PORTC|=(1<<7))
#define CLEARLED (PORTC&=~(1<<7))

/* ...hardware SPI, using SCK/MOSI/MISO hardware SPI pins and then user selected CS/IRQ/RST */
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

//Software SPI
Adafruit_SSD1351 tft = Adafruit_SSD1351(cs, dc, mosi, sclk, rst);

volatile uint32_t seconds = 0;
volatile uint8_t count = 0;

String notif = "";
String lastNotif = "";
String lastTime = "";

volatile bool notifChange = true;
volatile bool timeChange = true;

bool connected = false;

//Timer-Interrupt
ISR(TIMER1_OVF_vect)
{
  if(count++>243){
    count = 0;
    seconds++;

    if(seconds>=86400){
      seconds -= 86400;
    }

    if(seconds%60 == 0){
      timeChange = true;
    }
  }
}

inline void setNotification(String text)
{
  if(text != notif){
    lastNotif = notif;
    notif = text;

    notifChange = true;
  }
}

inline void clearNotification(){
  setNotification("");
}


void setup(void)
{
    tft.begin();
    tft.fillScreen(BLACK);

    //Led 13 as Output
    DDRC |= (1<<7);

    tft.setCursor(0,58);
    tft.setTextColor(BLUE);
    tft.setTextSize(2);
    tft.println("Starting");

    ble.setMode(BLUEFRUIT_MODE_COMMAND);

    if ( !ble.begin(VERBOSE_MODE) )
    {
    //error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
    }
    Serial.println( F("OK!") );

    if ( FACTORYRESET_ENABLE )
    {
    /* Perform a factory reset to make sure everything is in a known state */
    //Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ){
      //error(F("Couldn't factory reset"));
    }
    }

    ble.println("AT+GAPDEVNAME=Smartwatch");
    ble.waitForOK();

    /* Disable command echo from Bluefruit */
    ble.echo(false);

    tft.setCursor(0,58);
    tft.setTextColor(BLUE);
    tft.setTextSize(2);
    tft.println("Starting");

    tft.fillScreen(BLACK);
}

void loop()
{
    if(timeChange){
        String timeS = "";
        uint8_t hour = (uint8_t)(seconds/3600);
        if(hour<=9)
            timeS += "0";
        timeS += String(hour);

        timeS += ":";

        uint8_t minute = (uint8_t)((seconds%3600)/60);
        if(minute<=9)
            timeS += "0";
        timeS += String(minute);

        tft.setTextSize(3);

        tft.setCursor(19,19);
        tft.setTextColor(BLACK);
        tft.print(lastTime);
        tft.setCursor(19,19);
        tft.setTextColor(CLOCK_COLOR);
        tft.print(timeS);


        lastTime = timeS;

        timeChange = false;
    }


    if(notifChange){
        tft.setTextSize(1);

        tft.setCursor(0,58);
        tft.setTextColor(BLACK);
        tft.print(lastNotif);
        tft.setCursor(0,58);
        tft.setTextColor(NOTIF_COLOR);
        tft.print(notif);

        notifChange = false;
    }

    ble.setMode(BLUEFRUIT_MODE_COMMAND);
    if(ble.isConnected()){
        if(!connected){
            clearNotification();
            connected = true;
        }

        ble.setMode(BLUEFRUIT_MODE_DATA);
    }else{
        setNotification("Please connect Watch to your Phone.");
        connected = false;
    }

    if(connected){
        if(ble.available()){
            /*
             * Protocol:
             * Leading char, meaning and payload
             *      N | Notification        | payload is message
             *      C | Clear Notification  | no payload
             *      T | Time;               |  time in seconds, ascii encoded
             */
            switch(ble.read()){
                case 'N':
                    setNotification(ble.readString());
                break;
                case 'C':
                    clearNotification();
                break;
                case 'T':
                    //Disable interrupts (-> No timer interrupt and no periphery)
                    cli();
                    // Init Timer/Counter 1
                    TCCR1B = 0b011;    //Prescaler 1/64 (Page 108)
                    TCCR1C = 0b000000; //No Output (PWM)
                    TIMSK1 = 0b1 << 0; //Overflow-Interrupt
                    TCNT1 = 0;         //Set Timer to 0

                    seconds = 0;

                    delay(1000);
                    while(ble.available())
                    {
                        delay(1000);
                        seconds = seconds*10 + (ble.read() - '0');
                    }
                    count = 0;
                    timeChange = true;

                    //Enable interrupts
                    sei();
                break;
                default:
                    ble.println("Received unknown data");
                break;
      }
    }
  }
}
