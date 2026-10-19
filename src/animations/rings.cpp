#include "animations/rings.h"

void drawRing(Display& display, uint8_t radius) {
  // the 8x8 centre sits between pixels 3 and 4, radius r spans 3-r to 4+r
  const int8_t low = static_cast<int8_t>(3 - radius);
  const int8_t high = static_cast<int8_t>(4 + radius);

  for (int8_t i = low; i <= high; i++) {
    display.setPixel(i, low, true);
    display.setPixel(i, high, true);
    display.setPixel(low, i, true);
    display.setPixel(high, i, true);
  }
}
