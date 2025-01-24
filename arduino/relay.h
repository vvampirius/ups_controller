#include "battery.h"

class Relay {
  public:
    bool isOffDueToVoltage = false; // писать в энергнонезависимую память
    
    Relay(uint8_t pinOn, uint8_t pinOff, uint16_t offVoltage, uint32_t recoveryTimeout, uint16_t eepromCell);
    void checkBattery(Battery* battery);
    void hardReboot();
    void on();
    void off();
    void setIsOffDueToVolage(bool value);
  
  private:
    uint8_t _pinOn, _pinOff;
    uint16_t _offVoltage;
    uint32_t _recovertTimeout;
    uint16_t _eepromCell;
};
