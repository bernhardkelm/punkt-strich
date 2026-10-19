#include "animations/Limit.h"

#include <Arduino.h>

namespace {

// column bits for x=1 and x=6, the shoulders of the top and bottom rim rows
constexpr uint8_t kEdgePair = 0x42;
// column bits for x=2 and x=5, the neck-row edges and the outer sand pair
constexpr uint8_t kOuterPair = 0x24;
// column bits for x=3 and x=4, the neck itself and the inner sand pair
constexpr uint8_t kInnerPair = 0x18;
constexpr uint8_t kRim = 0xFF;

}

const char* Limit::name() const {
  return "limit";
}

void Limit::begin(Display& display) {
  startMs_ = millis();
  frame_ = 0;
  draw(display, frame_);
}

void Limit::tick(Display& display, uint32_t nowMs) {
  const uint32_t elapsed = (nowMs - startMs_) % (kFrameCount * kFrameMs);
  const uint8_t frame = static_cast<uint8_t>(elapsed / kFrameMs);

  if (frame == frame_) {
    return;
  }

  frame_ = frame;
  draw(display, frame);
}

uint8_t Limit::rowMask(uint8_t y, uint8_t level) {
  switch (y) {
    case 0:
    case 7:
      return kRim;
    case 1:
      return static_cast<uint8_t>(kEdgePair | (level >= 3 ? kOuterPair : 0) | (level >= 2 ? kInnerPair : 0));
    case 2:
      return static_cast<uint8_t>(kOuterPair | (level >= 1 ? kInnerPair : 0));
    case 5:
      return static_cast<uint8_t>(kOuterPair | (level <= 0 ? kInnerPair : 0));
    case 6:
      return static_cast<uint8_t>(kEdgePair | (level <= 2 ? kOuterPair : 0) | (level <= 1 ? kInnerPair : 0));
    default:
      return kInnerPair;
  }
}

void Limit::draw(Display& display, uint8_t frame) const {
  const uint8_t level = static_cast<uint8_t>(kLevelCount - 1 - frame / 2);

  display.setBrightness(maxBrightness());
  display.clear();

  for (uint8_t y = 0; y < Display::kHeight; y++) {
    const uint8_t row = rowMask(y, level);

    for (uint8_t x = 0; x < Display::kWidth; x++) {
      display.setPixel(x, y, row & (1 << x));
    }
  }

  display.show();
}
