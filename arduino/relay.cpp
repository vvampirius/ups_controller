#include "relay.h"
#include "getTimeDelta.h"
#include <EEPROM.h>

Relay::Relay(uint8_t pinOn, uint8_t pinOff, uint16_t offVoltage, uint32_t recoveryTimeout, uint16_t eepromCell) : _pinOn(pinOn),
    _pinOff(pinOff), _offVoltage(offVoltage), _recovertTimeout(recoveryTimeout), _eepromCell(eepromCell) {
    pinMode(pinOn, OUTPUT);
    pinMode(pinOff, OUTPUT);
    uint8_t x = EEPROM[eepromCell];
    if (x == 1) isOffDueToVoltage = true;
};

void Relay::checkBattery(Battery* battery) {
    if (isOffDueToVoltage) {
        if (!battery->isOnline) return;
        if (getTimeDelta(battery->onlineSince) < _recovertTimeout) return;
        on();
        setIsOffDueToVolage(false);
        return;
    }
    if (battery->voltage <= _offVoltage) {
        off();
        setIsOffDueToVolage(true);
        return;
    }
}

void Relay::hardReboot() {
    off();
    on();
}

void Relay::on() {
    digitalWrite(_pinOn, HIGH);
    delay(50);
    digitalWrite(_pinOn, LOW);
    Serial.print(_pinOn);
    Serial.println(" ->ON");
}

void Relay::off() {
    digitalWrite(_pinOff, HIGH);
    delay(50);
    digitalWrite(_pinOff, LOW);
    Serial.print(_pinOff);
    Serial.println(" ->OFF");
}

void Relay::setIsOffDueToVolage(bool value) {
    isOffDueToVoltage = value;
    EEPROM.update(_eepromCell, (uint8_t)value);
}