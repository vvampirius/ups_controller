#include "battery.h"
#include "ledBlink.h"
#include "button.h"

class RaspberryLed {
  public:
    uint8_t mode = 0; // 0 - none (normal), 1 - blink slow (shutdown/reboot), 2 - blink fast (unexpected !isAlive), 3 - light on PING

    RaspberryLed(uint8_t pin);
    void setMode(uint8_t value);
    void light(uint32_t duration);
    void update();

  private:
    uint8_t _pin;
    LedBlink* ledBlink;
    long lightStartedAt;
    uint32_t lightDuration;
};

class Raspberry {
  public:
    bool isRelayOn = false;
    uint8_t powerMode = 0; // 0 - normal (ON), 1 - manual shutdown, 2 - reboot, 3 - low voltage (OFF), 4 - reboot via ups. TODO: писать в энергнонезависимую память
    long shutdownStartedAt, pingCheckpointAt;
    RaspberryLed* raspberryLed;
    
    Raspberry(uint8_t pinOn, uint8_t pinOff, uint8_t pinShutdown, uint8_t pinCallbcak, uint8_t pinLed, uint8_t pinButton,
    uint16_t shutdownVoltage, uint32_t shutdownTimeout, uint32_t recoveryTimeout, uint16_t eepromCell);
    void checkBattery(Battery* battery);
    void checkButton();
    void checkPingAge();
    void checkShutdown();
    void hardReboot();
    bool isAlive();
    void on();
    void off();
    void reboot();
    void setPowerMode(uint8_t value);
    void shutdown();
    void updateLed();
  
  private:
    uint8_t _pinOn, _pinOff, _pinShutdown, _pinCallbcak;
    uint16_t _shutdownVoltage;
    uint32_t _shutdownTimeout, _recovertTimeout;
    uint8_t rebootState = 0; // 0 - not yet shutdown (needs to hard-reboot due timeout), 1 - not yet alive after shutdown (needs to hard-reboot due to timeout)
    Button* button;
    uint16_t _eepromCell;
};