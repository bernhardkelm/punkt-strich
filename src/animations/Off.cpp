#include "animations/Off.h"

#include "animations/blink.h"

#include <Arduino.h>

const char* Off::name() const {
  return "off";
}

void Off::begin(Display& display) {
  startMs_ = millis();
  lit_ = false;
  display.setBrightnessNow(kBlinkBrightness);
  display.clear();
  display.show();
}

void Off::tick(Display& display, uint32_t nowMs) {
  // the blink closes each period, so entering OFF stays dark for a full interval first
  const bool lit = blinkLitLast(nowMs - startMs_, kPeriodMs, kBlinkMs);

  if (lit == lit_) {
    return;
  }

  lit_ = lit;
  display.setBrightnessNow(kBlinkBrightness);
  display.clear();
  display.setPixel(Display::kWidth - 1, Display::kHeight - 1, lit);
  display.show();
}
