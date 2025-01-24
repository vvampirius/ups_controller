#pragma once
#include <Arduino.h>

class LedBlink {
  public:
    LedBlink(uint8_t pin, uint32_t lowDuration, uint32_t highDuration);
    void update();
  
  private:
    uint8_t state = LOW;
    uint8_t _pin;
    long stateChangedAt;
    uint32_t _lowDuration, _highDuration;
    void switchState();
};