#define NOTIF_COLOR r8g8b8Tor5g6b5(White, brightness)
#define CLOCK_COLOR r8g8b8Tor5g6b5(Red::P500, brightness)
#define BACKGROUND_COLOR r8g8b8Tor5g6b5(Black)

#define START_COLOR r8g8b8Tor5g6b5(White)
#define START_BACKGROUND r8g8b8Tor5g6b5(Black)

#include "label.hpp"

uint8_t brightness = 255;

Label labelClock(BACKGROUND_COLOR);
Label labelNotification(BACKGROUND_COLOR);
Label labelStart(START_BACKGROUND);

void initUi(){
    oled.begin();
    oled.fillScreen(Black);

    labelStart.setPosition(0, 80);
    labelStart.setColor(START_COLOR);
    labelStart.setSize(2);

    labelClock.setPosition(19,48);
    labelClock.setColor(CLOCK_COLOR);
    labelClock.setSize(3);

    labelNotification.setPosition(0, 84);
    labelNotification.setColor(NOTIF_COLOR);
    labelNotification.setSize(1);


}

void showStartScreen(){
    labelStart.setText("Starting");
    oled.println();
    oled.setTextColor(r8g8b8Tor5g6b5(Blue::P500));
    oled.print(".");
    oled.setTextColor(r8g8b8Tor5g6b5(Red::P500));
    oled.print(".");
    oled.setTextColor(r8g8b8Tor5g6b5(Yellow::P500));
    oled.print(".");
    oled.setTextColor(r8g8b8Tor5g6b5(Blue::P500));
    oled.print(".");
    oled.setTextColor(r8g8b8Tor5g6b5(Green::P500));
    oled.print(".");
    oled.setTextColor(r8g8b8Tor5g6b5(Red::P500));
    oled.print(".");
    oled.setTextColor(r8g8b8Tor5g6b5(Blue::P500));
    oled.print(".");
    oled.setTextColor(r8g8b8Tor5g6b5(Red::P500));
    oled.print(".");
    oled.setTextColor(r8g8b8Tor5g6b5(Yellow::P500));
    oled.print(".");
    oled.setTextColor(r8g8b8Tor5g6b5(Blue::P500));
    oled.print(".");
}

void updateClock(){
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

    labelClock.setText(timeS);
}

void updateNotif(){
    labelNotification.setText(notif);
}

void forceRedraw(){
    labelClock.setColor(CLOCK_COLOR);
    labelNotification.setColor(NOTIF_COLOR);

    labelClock.update();
    labelNotification.update();
}

void drawLabel(String text, uint8_t x, uint8_t y, uint8_t size, uint16_t color){
    oled.setTextSize(size);

    oled.setCursor(x,y);
    oled.setTextColor(color);
    oled.print(text);
}



