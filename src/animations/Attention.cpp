#include "animations/Attention.h"

#include "animations/rings.h"

#include <Arduino.h>

const char* Attention::name() const {
  return "attention";
}

void Attention::begin(Display& display) {
  startMs_ = millis();
  frame_ = 0;
  draw(display, frame_);
}

void Attention::tick(Display& display, uint32_t nowMs) {
  const uint8_t frame = frameAt((nowMs - startMs_) % kPeriodMs);

  if (frame == frame_) {
    return;
  }

  frame_ = frame;
  draw(display, frame);
}

uint8_t Attention::frameAt(uint32_t phase) {
  if (phase >= kDarkStartMs) {
    return kRingCount;
  }

  // the outermost ring already covers all 64 pixels, so it also serves as the full frame
  const uint32_t ring = phase / kRingMs;
  return static_cast<uint8_t>(ring < kRingCount ? ring : kRingCount - 1);
}

void Attention::draw(Display& display, uint8_t frame) const {
  display.setBrightness(maxBrightness());
  display.clear();

  if (frame < kRingCount) {
    for (uint8_t radius = 0; radius <= frame; radius++) {
      drawRing(display, radius);
    }
  }

  display.show();
}
