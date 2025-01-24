#include "raspberry.h"
#include "getTimeDelta.h"
#include <EEPROM.h>

Raspberry::Raspberry(uint8_t pinOn, uint8_t pinOff, uint8_t pinShutdown, uint8_t pinCallbcak, uint8_t pinLed, uint8_t pinButton, uint16_t shutdownVoltage, uint32_t shutdownTimeout,
    uint32_t recoveryTimeout, uint16_t eepromCell) : _pinOn(pinOn), _pinOff(pinOff), _pinShutdown(pinShutdown), _pinCallbcak(pinCallbcak),
    _shutdownVoltage(shutdownVoltage), _shutdownTimeout(shutdownTimeout), _recovertTimeout(recoveryTimeout), _eepromCell(eepromCell) {
    pinMode(pinOn, OUTPUT);
    pinMode(pinOff, OUTPUT);
    pinMode(pinShutdown, OUTPUT);
    pinMode(pinCallbcak, INPUT);
    pinMode(pinLed, OUTPUT);
    powerMode = EEPROM[eepromCell];
    if (powerMode > 3) powerMode = 0;
    raspberryLed = new RaspberryLed(pinLed);
    button = new Button(pinButton);
};

void Raspberry::checkBattery(Battery* battery) {
    switch (powerMode) {
        case 0:
            if (battery->voltage <= _shutdownVoltage) {
                setPowerMode(3);
                if (isRelayOn) shutdown();
            }
            break;
        case 3: // restore power
            if (isRelayOn) break; // looks like is still shutdown
            if (!battery->isOnline) break;
            if (getTimeDelta(battery->onlineSince) < _recovertTimeout) break;
            setPowerMode(0);
            on();
            break;
    }
}

void Raspberry::checkButton() {
    uint32_t duration = button->pressed();
    if (duration >= 3000) {
        setPowerMode(1);
        off();
        return;
    }
    if (duration > 0) {
        if (!isRelayOn) {
            if (powerMode == 3) {
                raspberryLed->light(1000);
                return;
            }
            setPowerMode(0);
            on();
            return;
        }
        if (powerMode == 0) {
            setPowerMode(1);
            shutdown();
        }
    }
}

void Raspberry::checkPingAge() {
    if (powerMode != 0) return;
    if (getTimeDelta(pingCheckpointAt) >= 14400000) { // 4h
        Serial.println("Trying to reboot due to PING abscence");
        pingCheckpointAt = millis();
        reboot();
    }
}

void Raspberry::checkShutdown() {
    if (!isRelayOn) return;
    switch (powerMode) {
        case 1:
            if (!isAlive()) {
                Serial.println("Shutdown due to callback");
                off();
            }
            if (getTimeDelta(shutdownStartedAt) >= _shutdownTimeout) {
                Serial.println("Shutdown due to timeout");
                off();
            }
            break;
        case 2:
            if (rebootState == 0) {
                if (!isAlive()) {
                    Serial.println("reboot state->1 due to callbcak");
                    rebootState = 1;
                }
                if (getTimeDelta(shutdownStartedAt) >= _shutdownTimeout) {
                    Serial.println("reboot state->1 with hard-reboot due to timeout");
                    rebootState = 1;
                    hardReboot();
                }
            } else if (rebootState == 1) {
                if (isAlive()) {
                    Serial.println("reboot complete due to callbcak");
                    powerMode = 0;
                    rebootState = 0;
                }
                if (getTimeDelta(shutdownStartedAt) >= _shutdownTimeout * 3) {
                    Serial.println("hard-reboot due to timeout");
                    powerMode = 0;
                    rebootState = 0;
                    hardReboot();
                }
            }
            break;
        case 3:
            if (!isAlive()) {
                Serial.println("Shutdown due to callback");
                off();
            }
            if (getTimeDelta(shutdownStartedAt) >= _shutdownTimeout) {
                Serial.println("Shutdown due to timeout");
                off();
            }
            break;
        case 4:
            if (!isAlive()) {
                Serial.println("Shutdown due to callback");
                rebootState = 1;
            }
            if (getTimeDelta(shutdownStartedAt) >= _shutdownTimeout) {
                Serial.println("Shutdown due to timeout");
                rebootState = 1;
            }
            if (rebootState == 1) {
                rebootState = 0;
                powerMode = 0;
                hardReboot();
            }
            break;
    }
}

void Raspberry::hardReboot() {
    off();
    on();
}

bool Raspberry::isAlive() {
    return !digitalRead(_pinCallbcak);
}

void Raspberry::on() {
    digitalWrite(_pinOn, HIGH);
    delay(50);
    digitalWrite(_pinOn, LOW);
    isRelayOn = true;
    pingCheckpointAt = millis();
    Serial.print(_pinOn);
    Serial.println(" ->ON");
}

void Raspberry::off() {
    digitalWrite(_pinOff, HIGH);
    delay(50);
    digitalWrite(_pinOff, LOW);
    isRelayOn = false;
    Serial.print(_pinOff);
    Serial.println(" ->OFF");
}

void Raspberry::reboot() {
    powerMode = 4;
    shutdown();
}

void Raspberry::setPowerMode(uint8_t value) {
    if (value == powerMode) return;
    powerMode = value;
    if (powerMode == 0 || powerMode == 1 || powerMode == 3) EEPROM.update(_eepromCell, powerMode);
}

void Raspberry::shutdown() {
    digitalWrite(_pinShutdown, HIGH);
    Serial.println("Shutdown");
    delay(200);
    digitalWrite(_pinShutdown, LOW);
    shutdownStartedAt = millis();
}

void Raspberry::updateLed() {
    switch (powerMode) {
        case 0:
            if (!isAlive()) {
                raspberryLed->setMode(2);
                raspberryLed->update();
            } else raspberryLed->setMode(0);
            break;
        case 1:
            if (isRelayOn) {
                raspberryLed->setMode(1);
                raspberryLed->update();
            } else raspberryLed->setMode(0);
            break;
        case 3:
            if (isRelayOn) {
                raspberryLed->setMode(1);
                raspberryLed->update();
            } else if (raspberryLed->mode == 3) {
                raspberryLed->update();
            } else raspberryLed->setMode(0);
            break;
        default:
            raspberryLed->setMode(1);
            raspberryLed->update();
            break;
    }
}


RaspberryLed::RaspberryLed(uint8_t pin) : _pin(pin) {}

void RaspberryLed::setMode(uint8_t value) {
    if (value == mode) return;
    switch (value) {
        case 0:
            digitalWrite(_pin, LOW);
            break;
        case 1:
            if (ledBlink != NULL) delete ledBlink;
            ledBlink = new LedBlink(_pin, 600, 600);
            break;
        case 2:
            if (ledBlink != NULL) delete ledBlink;
            ledBlink = new LedBlink(_pin, 100, 100);
            break;
    }
    mode = value;
}

void RaspberryLed::light(uint32_t duration) {
    if (mode != 0) return;
    mode = 3;
    lightDuration = duration;
    lightStartedAt = millis();
    digitalWrite(_pin, HIGH);
}

void RaspberryLed::update() {
    if (mode == 0) return;
    if (mode == 3) {
        if (getTimeDelta(lightStartedAt) >= lightDuration) setMode(0);
        return;
    }
    ledBlink->update();
}