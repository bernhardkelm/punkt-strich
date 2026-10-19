#include "clockFaces/scaledDigit.h"

#include "animations/digitFont.h"

void drawScaledDigit(Display& display, uint8_t digit, int8_t x, int8_t y, uint8_t rows) {
  for (uint8_t row = 0; row < rows; row++) {
    const uint8_t bits = digitRow(digit, static_cast<uint8_t>(row * kDigitRows / rows));

    for (uint8_t col = 0; col < kDigitCols; col++) {
      if (((bits >> (kDigitCols - 1 - col)) & 1) == 0) {
        continue;
      }

      display.setCanvasPixel(static_cast<int8_t>(x + col), static_cast<int8_t>(y + row), true);
    }
  }
}
