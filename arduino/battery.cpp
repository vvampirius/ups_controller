#include "battery.h"

Battery::Battery(uint8_t pin = A0) : _pin(pin) {}

void Battery::read() {
    rawValue = analogRead(_pin);
    voltage = constrain(map(rawValue, 0, 1023, 0, 1410), 0, 1500); // TODO: move 1410 to header file
    if (voltage < 1330) { // 13.3V
        isOnline = false;
    } else {
        if (!isOnline) onlineSince = millis();
        isOnline = true;
    }
}