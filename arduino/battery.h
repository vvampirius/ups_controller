#pragma once
#include <Arduino.h>

class Battery {
  public:
    bool isOnline = false;
    long onlineSince = 0;
    int rawValue = 0;
    uint16_t voltage = 0;
    
    Battery(uint8_t pin = A0);
    void read();
  
  private:
    uint8_t _pin;
};
