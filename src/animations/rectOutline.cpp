#include "animations/rectOutline.h"

void drawRectOutline(Display& display, int8_t x0, int8_t y0, int8_t x1, int8_t y1) {
  const int8_t left = x0 < x1 ? x0 : x1;
  const int8_t right = x0 < x1 ? x1 : x0;
  const int8_t top = y0 < y1 ? y0 : y1;
  const int8_t bottom = y0 < y1 ? y1 : y0;

  for (int8_t x = left; x <= right; x++) {
    display.setPixel(x, top, true);
    display.setPixel(x, bottom, true);
  }

  for (int8_t y = top; y <= bottom; y++) {
    display.setPixel(left, y, true);
    display.setPixel(right, y, true);
  }
}
