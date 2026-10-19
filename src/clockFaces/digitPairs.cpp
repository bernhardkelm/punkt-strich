#include "clockFaces/digitPairs.h"

#include "animations/digitFont.h"
#include "clockFaces/glyphFrame.h"
#include "clockFaces/smallDigit.h"

namespace {

constexpr uint8_t kTens = 10;
constexpr uint8_t kPairWidth = kDigitPairStep + kDigitCols;

/**
 * Returns one glyph row of a digit pair, the tens digit's row left of the ones digit's.
 * @param {uint8_t} tens - glyph row of the tens digit, three low bits
 * @param {uint8_t} ones - glyph row of the ones digit, three low bits
 * @returns {uint8_t} - kPairWidth low bits, the highest one is the left column
 */
uint8_t pairRow(uint8_t tens, uint8_t ones) {
  return static_cast<uint8_t>((tens << kDigitPairStep) | ones);
}

}

Frame digitPairFrame(uint8_t value, int8_t x, int8_t y) {
  const uint8_t tens = static_cast<uint8_t>(value / kTens);
  const uint8_t ones = static_cast<uint8_t>(value % kTens);
  uint8_t rows[kDigitRows];

  for (uint8_t row = 0; row < kDigitRows; row++) {
    rows[row] = pairRow(digitRow(tens, row), digitRow(ones, row));
  }

  return glyphFrame(rows, kDigitRows, kPairWidth, x, y);
}

Frame smallPairFrame(uint8_t value, int8_t x, int8_t y) {
  const uint8_t tens = static_cast<uint8_t>(value / kTens);
  const uint8_t ones = static_cast<uint8_t>(value % kTens);
  uint8_t rows[kSmallDigitRows];

  for (uint8_t row = 0; row < kSmallDigitRows; row++) {
    rows[row] = pairRow(smallDigitRow(tens, row), smallDigitRow(ones, row));
  }

  return glyphFrame(rows, kSmallDigitRows, kPairWidth, x, y);
}
