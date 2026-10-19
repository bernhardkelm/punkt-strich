#include "animations/Knight.h"

#include <Arduino.h>

const char* Knight::name() const {
  return "knight";
}

void Knight::begin(Display& display) {
  startMs_ = millis();
  left_ = 0;
  movingRight_ = true;
  draw(display, left_, movingRight_);
}

void Knight::tick(Display& display, uint32_t nowMs) {
  const uint32_t phase = (nowMs - startMs_) % kCycleMs;
  const uint32_t half = kCycleMs / 2;
  const bool movingRight = phase < half;
  const uint32_t rising = movingRight ? phase : kCycleMs - phase;
  const int8_t left = static_cast<int8_t>((rising * kMaxLeft + half / 2) / half);

  if (left == left_ && movingRight == movingRight_) {
    return;
  }

  left_ = left;
  movingRight_ = movingRight;
  draw(display, left, movingRight);
}

void Knight::draw(Display& display, int8_t left, bool movingRight) const {
  display.setBrightness(maxBrightness());
  display.clear();

  // the trail sits one column behind the block, opposite the direction of travel
  const int8_t trail = movingRight ? static_cast<int8_t>(left - 1)
                                   : static_cast<int8_t>(left + kBlockSize);

  for (uint8_t row = 0; row < kBlockSize; row++) {
    const int8_t y = static_cast<int8_t>(kTopRow + row);

    for (uint8_t column = 0; column < kBlockSize; column++) {
      display.setPixel(static_cast<int8_t>(left + column), y, true);
    }

    display.setPixel(trail, y, true);
  }

  display.show();
}
