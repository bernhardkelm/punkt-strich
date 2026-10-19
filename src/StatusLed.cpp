#include "StatusLed.h"

#include <Arduino.h>

namespace {

constexpr uint8_t kPin = 48;

}

namespace StatusLed {

void off() {
  neopixelWrite(kPin, 0, 0, 0);
}

}
