#include "ledBlink.h"
#include "getTimeDelta.h"

LedBlink::LedBlink(uint8_t pin, uint32_t lowDuration, uint32_t highDuration) : _pin(pin), _lowDuration(lowDuration), _highDuration(highDuration) {
  digitalWrite(pin, state);
  stateChangedAt = millis();
}

void LedBlink::update() {
  if (state == LOW && getTimeDelta(stateChangedAt) >= _lowDuration) switchState();
  if (state == HIGH && getTimeDelta(stateChangedAt) >= _highDuration) switchState();
}

void LedBlink::switchState() {
  if (state == LOW) {
    state = HIGH;
  } else state = LOW;
  digitalWrite(_pin, state);
  stateChangedAt = millis();
}