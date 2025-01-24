#pragma once
#include "getTimeDelta.h"

class Button {
    public:
        Button(uint8_t pin);
        uint32_t pressed();
    
    private:
        uint8_t _pin;
        bool _pressed = false;
        long pressStartedAt;
};