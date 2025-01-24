#include "button.h"

Button::Button(uint8_t pin) : _pin(pin) {
    pinMode(pin, INPUT_PULLUP);
}

uint32_t Button::pressed() {
    int state = digitalRead(_pin);
    switch (_pressed) {
        case false:
            if (state == HIGH) return 0; // not yet pressed
            _pressed = true;
            pressStartedAt = millis();
            break;
        case true:
            if (state == LOW) return 0; // still pressed
            uint32_t duration = getTimeDelta(pressStartedAt);
            if (duration < 50) return 0; // too fast
            _pressed = false;
            return duration;
            break;
    }
    return 0;
}