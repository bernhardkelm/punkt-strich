#include "animations/glitchFrame.h"

#include <Arduino.h>

namespace {

constexpr uint8_t kAntiDiagonal = Display::kWidth - 1;
constexpr uint8_t kGlitchRowCount = 3;
constexpr int8_t kGlitchShifts[kGlitchRowCount] = {-2, 0, 2};
constexpr uint8_t kGlitchPixels = 6;

/**
 * Returns the pixel mask of the plain thick X for one row.
 * @param {uint8_t} y - row index
 * @returns {uint8_t} - one bit per lit column
 */
uint8_t xRow(uint8_t y) {
  uint8_t row = 0;

  for (uint8_t x = 0; x < Display::kWidth; x++) {
    if (x == y || x == y + 1 || x + y == kAntiDiagonal || x + y == kAntiDiagonal + 1) {
      row = static_cast<uint8_t>(row | (1 << x));
    }
  }

  return row;
}

}

void xRows(uint8_t (&rows)[Display::kHeight]) {
  for (uint8_t y = 0; y < Display::kHeight; y++) {
    rows[y] = xRow(y);
  }
}

void generateGlitchRows(const uint8_t (&base)[Display::kHeight], uint8_t (&rows)[Display::kHeight]) {
  for (uint8_t y = 0; y < Display::kHeight; y++) {
    rows[y] = base[y];
  }

  for (uint8_t i = 0; i < kGlitchRowCount; i++) {
    const uint8_t y = static_cast<uint8_t>(random(Display::kHeight));
    const int8_t shift = kGlitchShifts[i];

    if (shift > 0) {
      rows[y] = static_cast<uint8_t>(rows[y] << shift);
    } else if (shift < 0) {
      rows[y] = static_cast<uint8_t>(rows[y] >> -shift);
    }
  }

  for (uint8_t i = 0; i < kGlitchPixels; i++) {
    const uint8_t x = static_cast<uint8_t>(random(Display::kWidth));
    const uint8_t y = static_cast<uint8_t>(random(Display::kHeight));
    rows[y] = static_cast<uint8_t>(rows[y] ^ (1 << x));
  }
}

void drawRows(Display& display, uint8_t brightness, const uint8_t (&rows)[Display::kHeight]) {
  display.setBrightnessNow(brightness);
  display.clear();

  for (uint8_t y = 0; y < Display::kHeight; y++) {
    for (uint8_t x = 0; x < Display::kWidth; x++) {
      display.setPixel(x, y, rows[y] & (1 << x));
    }
  }

  display.show();
}
