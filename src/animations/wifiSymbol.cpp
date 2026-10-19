#include "animations/wifiSymbol.h"

#include <cmath>

namespace {

constexpr int8_t kOffsetX = (Display::kCanvasWidth - Display::kWidth) / 2;
constexpr int8_t kDotX = 0;
constexpr int8_t kDotY = Display::kHeight - 1;
constexpr uint8_t kArcRadius[kWifiArcCount] = {2, 4, 6};

/**
 * Returns how far a pixel sits from the dot, rounded to whole pixels.
 * @param {int8_t} x - column, 0 is left
 * @param {int8_t} y - row, 0 is top
 * @returns {uint8_t} - rounded Euclidean distance
 */
uint8_t distanceFromDot(int8_t x, int8_t y) {
  const float dx = static_cast<float>(x - kDotX);
  const float dy = static_cast<float>(y - kDotY);
  return static_cast<uint8_t>(lroundf(sqrtf(dx * dx + dy * dy)));
}

}

void drawWifiSymbol(Display& display, uint8_t arcs) {
  const uint8_t lit = arcs > kWifiArcCount ? kWifiArcCount : arcs;
  display.setCanvasPixel(kOffsetX + kDotX, kDotY, true);

  for (int8_t y = 0; y < static_cast<int8_t>(Display::kHeight); y++) {
    for (int8_t x = 0; x < static_cast<int8_t>(Display::kWidth); x++) {
      const uint8_t distance = distanceFromDot(x, y);

      for (uint8_t arc = 0; arc < lit; arc++) {
        if (distance == kArcRadius[arc]) {
          display.setCanvasPixel(static_cast<int8_t>(kOffsetX + x), y, true);
        }
      }
    }
  }
}
