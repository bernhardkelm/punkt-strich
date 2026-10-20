#include "animations/Unread.h"

#include "animations/marker.h"

#include <Arduino.h>

const char* Unread::name() const {
  return "unread";
}

void Unread::begin(Display& display) {
  startMs_ = millis();
  lit_ = true;
  drawMarker(display, lit_, maxBrightness(), contextPercent());
}

void Unread::tick(Display& display, uint32_t nowMs) {
  const bool lit = markerLit(nowMs - startMs_);

  if (lit == lit_) {
    return;
  }

  lit_ = lit;
  drawMarker(display, lit_, maxBrightness(), contextPercent());
}

void Unread::applyBrightness(Display& display, uint32_t fadeMs) {
  display.setBrightness(markerBrightness(maxBrightness()), fadeMs);
}
