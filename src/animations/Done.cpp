#include "animations/Done.h"

#include "animations/rings.h"

#include <Arduino.h>

const char* Done::name() const {
  return "done";
}

void Done::begin(Display& display) {
  startMs_ = millis();
  frame_ = 0;
  finished_ = false;
  draw(display, frame_);
}

void Done::tick(Display& display, uint32_t nowMs) {
  const uint32_t elapsed = nowMs - startMs_;

  if (elapsed >= kDurationMs) {
    finished_ = true;
  }

  const uint8_t frame = frameAt(elapsed);

  if (frame == frame_) {
    return;
  }

  frame_ = frame;
  draw(display, frame);
}

bool Done::finished() const {
  return finished_;
}

uint8_t Done::frameAt(uint32_t elapsed) {
  if (elapsed < kDimMs) {
    return static_cast<uint8_t>(elapsed / kRingMs);
  }

  if (elapsed < kPeakMs) {
    return kRingCount;
  }

  if (elapsed < kHoldMs) {
    return kRingCount + 1;
  }

  if (elapsed < kDrainStartMs) {
    return kRingCount + 2;
  }

  if (elapsed < kDurationMs) {
    return static_cast<uint8_t>(kDrainFrame + (elapsed - kDrainStartMs) / kDrainRowMs);
  }

  return kEndFrame;
}

uint8_t Done::scaled(uint8_t percent) const {
  return static_cast<uint8_t>((maxBrightness() * percent + 50) / 100);
}

void Done::draw(Display& display, uint8_t frame) const {
  display.clear();

  if (frame < kRingCount) {
    display.setBrightness(maxBrightness());

    for (uint8_t radius = 0; radius <= frame; radius++) {
      drawRing(display, radius);
    }

    display.show();
    return;
  }

  if (frame == kRingCount) {
    display.setBrightness(scaled(kDimPercent));
  } else if (frame == kRingCount + 1) {
    display.setBrightness(scaled(kPeakPercent));
  } else {
    display.setBrightness(scaled(kHoldPercent));
  }

  const uint8_t firstRow = frame < kDrainFrame ? 0 : static_cast<uint8_t>(frame - kDrainFrame + 1);

  for (uint8_t y = firstRow; y < Display::kHeight; y++) {
    for (uint8_t x = 0; x < Display::kWidth; x++) {
      display.setPixel(x, y, true);
    }
  }

  display.show();
}
