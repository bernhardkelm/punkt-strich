#include "animations/WifiSearch.h"

#include <Arduino.h>

const char* WifiSearch::name() const {
  return "wifi-search";
}

void WifiSearch::begin(Display& display) {
  startMs_ = millis();
  frame_ = 0;
  draw(display, frame_);
}

void WifiSearch::tick(Display& display, uint32_t nowMs) {
  const uint8_t frame = frameAt((nowMs - startMs_) % kPeriodMs);

  if (frame == frame_) {
    return;
  }

  frame_ = frame;
  draw(display, frame);
}

uint8_t WifiSearch::frameAt(uint32_t phase) {
  const uint32_t step = phase / kArcStepMs;

  if (step >= kStepCount) {
    return kStepCount;
  }

  return static_cast<uint8_t>(step);
}

void WifiSearch::draw(Display& display, uint8_t frame) const {
  display.setBrightness(maxBrightness());
  display.clearCanvas();

  if (frame < kStepCount) {
    drawWifiSymbol(display, frame);
  }

  display.show();
}
