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

// Hardware SPI
#define BLUEFRUIT_SPI_CS               8
#define BLUEFRUIT_SPI_IRQ              7
#define BLUEFRUIT_SPI_RST              4
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

// Software SPI
#define OLED_SCLK 2
#define OLED_MOSI 3
#define OLED_DC   5
#define OLED_CS   6
#define OLED_RST  9
Adafruit_SSD1351 oled = Adafruit_SSD1351(OLED_CS, OLED_DC, OLED_MOSI, OLED_SCLK, OLED_RST);


volatile uint32_t seconds = 0;
volatile uint8_t count = 0;

String notif = "";

bool connected = false;

#include "colorConvert.h"
#include "materialColors.h"
#include "ui.h"

//Timer-Interrupt
ISR(TIMER1_OVF_vect)
{
    if(count++>242){
        count = 0;
        seconds++;

        if(seconds>=86400){
            seconds -= 86400;
            updateClock();
        }

        if(seconds%60 == 0){
            updateClock();
        }
    }
}

inline void setNotification(String text)
{
    notif = text;

    updateNotif();
}

inline void clearNotification(){
    setNotification("");
}


void setup(void)
{
    initUi();
    showStartScreen();

    //Led 13 as Output
    DDRC |= (1<<7);
    
    ble.setMode(BLUEFRUIT_MODE_COMMAND);


    if ( !ble.begin(VERBOSE_MODE) ){
      //error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
    }
    

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

    oled.fillScreen(BACKGROUND_COLOR);
}

void loop()
{
    ble.setMode(BLUEFRUIT_MODE_COMMAND);
    if(ble.isConnected()){
        if(!connected){
            clearNotification();
            updateClock();
            connected = true;
        }

        ble.setMode(BLUEFRUIT_MODE_DATA);

        if(ble.available()){
            /*
             * Protocol:
             * Leading char, meaning and payload
             *      N | Notification        | payload is message
             *      C | Clear Notification  | no payload
             *      T | Time                | time in seconds, ascii encoded
             *      B | Brighness           | payload is brightness (one byte)
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
                    updateClock();

                    //Enable interrupts
                    sei();
                    break;
                case 'B':
                    brightness = (uint8_t)ble.read();
                    forceRedraw();
                    break;
                default:
                    ble.println("Received unknown data");
                    break;
            }
        }
    }else{
        setNotification("Please connect Watch to your Phone.");
        connected = false;
    }
}
