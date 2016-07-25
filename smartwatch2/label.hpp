//
// Created by paul on 25.07.16.
//

#ifndef SMARTWATCH2_LABEL_HPP
#define SMARTWATCH2_LABEL_HPP

#include "Arduino.h"

extern void drawLabel(String text, uint8_t x, uint8_t y, uint8_t size, uint16_t color);


class Label
{
    public:
        Label();
        Label(uint16_t background);
        void setPosition(uint8_t x, uint8_t y);
        void setText(String text);
        void clear();
        void setColor(uint16_t color);
        void setSize(uint8_t size);
        void update();

    private:
        void forceText(String text);
        String text;
        uint8_t x, y;
        uint16_t color;
        uint8_t size;
        uint16_t background;
};

#endif //SMARTWATCH2_LABEL_HPP
