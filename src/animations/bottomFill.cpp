#include "animations/bottomFill.h"

void setBottomFillPixel(Display& display, uint8_t n, bool on) {
  const int8_t x = static_cast<int8_t>(n % Display::kWidth);
  const int8_t y = static_cast<int8_t>(Display::kHeight - 1 - n / Display::kWidth);
  display.setPixel(x, y, on);
}

void setCanvasBottomFillPixel(Display& display, uint8_t n, bool on) {
  const int8_t x = static_cast<int8_t>(n % Display::kCanvasWidth);
  const int8_t y = static_cast<int8_t>(Display::kHeight - 1 - n / Display::kCanvasWidth);
  display.setCanvasPixel(x, y, on);
}
