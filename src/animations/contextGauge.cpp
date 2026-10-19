#include "animations/contextGauge.h"

#include <cmath>

void drawContextGauge(Display& display, uint8_t percent) {
  const uint8_t lit = static_cast<uint8_t>(lroundf(static_cast<float>(percent) * Display::kWidth / 100.0f));

  for (uint8_t x = 0; x < Display::kWidth; x++) {
    display.setPixel(x, Display::kHeight - 1, x < lit);
  }
}

uint8_t usableRows(uint8_t percent) {
  return percent > 0 ? Display::kHeight - 1 : Display::kHeight;
}
