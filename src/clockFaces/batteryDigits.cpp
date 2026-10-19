#include "clockFaces/batteryDigits.h"

#include "animations/digitFont.h"
#include "clockFaces/batteryOutline.h"
#include "clockFaces/glyphFrame.h"

namespace {

constexpr uint8_t kTens = 10;
constexpr uint8_t kGap = 1;
constexpr uint8_t kTensShift = kDigitCols + kGap;
constexpr uint8_t kPairWidth = kTensShift + kDigitCols;
constexpr int8_t kTop = kBatteryInsideTop + (kBatteryInsideHeight - kDigitRows) / 2;

}

Frame batteryDigits(uint8_t percent) {
  const uint8_t shown = percent > kBatteryMaxShownPercent ? kBatteryMaxShownPercent : percent;
  const bool pair = shown >= kTens;
  const uint8_t width = pair ? kPairWidth : kDigitCols;
  const int8_t left = static_cast<int8_t>(kBatteryInsideLeft + (kBatteryInsideWidth - width) / 2);
  uint8_t rows[kDigitRows];

  for (uint8_t row = 0; row < kDigitRows; row++) {
    const uint8_t tens = pair ? digitRow(static_cast<uint8_t>(shown / kTens), row) : 0;
    const uint8_t ones = digitRow(static_cast<uint8_t>(shown % kTens), row);
    rows[row] = static_cast<uint8_t>((tens << kTensShift) | ones);
  }

  return glyphFrame(rows, kDigitRows, width, left, kTop);
}
