#include "animations/skullFrame.h"

namespace {

/**
 * Returns the pixel mask of a bitmap row written the way it lights up.
 * @param {uint8_t} bitmap - row with bit 7 as the left column
 * @returns {uint8_t} - pixel mask with bit x lighting column x
 */
constexpr uint8_t maskOf(uint8_t bitmap) {
  uint8_t mask = 0;

  for (uint8_t x = 0; x < Display::kWidth; x++) {
    if ((bitmap >> (Display::kWidth - 1 - x)) & 1) {
      mask = static_cast<uint8_t>(mask | (1 << x));
    }
  }

  return mask;
}

constexpr uint8_t kSkull[Display::kHeight] = {
  maskOf(0b00111100),
  maskOf(0b01111110),
  maskOf(0b11011011),
  maskOf(0b11111111),
  maskOf(0b01111110),
  maskOf(0b00101000),
  maskOf(0b00111100),
  maskOf(0b00101000),
};

}

void skullRows(uint8_t (&rows)[Display::kHeight]) {
  for (uint8_t y = 0; y < Display::kHeight; y++) {
    rows[y] = kSkull[y];
  }
}
