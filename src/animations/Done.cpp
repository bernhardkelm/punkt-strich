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

void Done::applyBrightness(Display& display, uint32_t fadeMs) {
  display.setBrightness(levelFor(frame_), fadeMs);
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

uint8_t Done::levelFor(uint8_t frame) const {
  if (frame < kRingCount) {
    return maxBrightness();
  }

  if (frame == kRingCount) {
    return scaled(kDimPercent);
  }

  if (frame == kRingCount + 1) {
    return scaled(kPeakPercent);
  }

  return scaled(kHoldPercent);
}

void Done::draw(Display& display, uint8_t frame) const {
  display.clear();
  display.setBrightness(levelFor(frame));

  if (frame < kRingCount) {
    for (uint8_t radius = 0; radius <= frame; radius++) {
      drawRing(display, radius);
    }

    display.show();
    return;
  }

  const uint8_t firstRow = frame < kDrainFrame ? 0 : static_cast<uint8_t>(frame - kDrainFrame + 1);

  for (uint8_t y = firstRow; y < Display::kHeight; y++) {
    for (uint8_t x = 0; x < Display::kWidth; x++) {
      display.setPixel(x, y, true);
    }
  }

  display.show();
}
