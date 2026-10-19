#include "animations/WifiLost.h"

#include "animations/blink.h"
#include "animations/wifiSymbol.h"

#include <Arduino.h>

const char* WifiLost::name() const {
  return "wifi-lost";
}

void WifiLost::begin(Display& display) {
  startMs_ = millis();
  lit_ = true;
  draw(display, lit_);
}

void WifiLost::tick(Display& display, uint32_t nowMs) {
  const bool lit = blinkLitFirst(nowMs - startMs_, kPeriodMs, kOnMs);

  if (lit == lit_) {
    return;
  }

  lit_ = lit;
  draw(display, lit);
}

void WifiLost::draw(Display& display, bool lit) const {
  display.setBrightness(maxBrightness());
  display.clearCanvas();

  if (lit) {
    drawWifiSymbol(display, kWifiArcCount);
  }

  display.show();
}
