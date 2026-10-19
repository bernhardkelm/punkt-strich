#include "animations/digitFont.h"

namespace {

constexpr uint8_t kDigitCount = 10;

// every pair of glyphs differs in at least two pixels, 8 is waisted so it cannot read as 0
constexpr uint8_t kFont[kDigitCount][kDigitRows] = {
  {0b111, 0b101, 0b101, 0b111},
  {0b110, 0b010, 0b010, 0b111},
  {0b111, 0b001, 0b110, 0b111},
  {0b111, 0b011, 0b001, 0b111},
  {0b101, 0b111, 0b001, 0b001},
  {0b111, 0b100, 0b011, 0b111},
  {0b100, 0b111, 0b101, 0b111},
  {0b111, 0b001, 0b010, 0b010},
  {0b111, 0b010, 0b101, 0b111},
  {0b111, 0b101, 0b111, 0b001},
};

}

uint8_t digitRow(uint8_t digit, uint8_t row) {
  if (digit >= kDigitCount || row >= kDigitRows) {
    return 0;
  }

  return kFont[digit][row];
}

void drawDigit(Display& display, uint8_t digit, int8_t x, int8_t y) {
  if (digit >= kDigitCount) {
    return;
  }

  for (uint8_t row = 0; row < kDigitRows; row++) {
    const uint8_t bits = digitRow(digit, row);

    for (uint8_t col = 0; col < kDigitCols; col++) {
      if (((bits >> (kDigitCols - 1 - col)) & 1) == 0) {
        continue;
      }

      display.setPixel(static_cast<int8_t>(x + col), static_cast<int8_t>(y + row), true);
    }
  }
}
