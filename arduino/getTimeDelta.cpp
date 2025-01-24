#include "getTimeDelta.h"

uint32_t getTimeDelta(uint32_t since, uint32_t now) {
  uint32_t delta;
  if (since > now) {
    delta = UINT32_MAX - since + now;
  } else {
    delta = now - since;
  }
  return delta;
}

uint32_t getTimeDelta(uint32_t since) {
  return getTimeDelta(since, millis());
}