#include "clockFaces/clockFrame.h"

#include "clockFaces/scaledDigit.h"

namespace {

constexpr int8_t kHourTensX = 0;
constexpr int8_t kHourOnesX = 4;
constexpr int8_t kMinuteOnesX = 13;
constexpr uint8_t kTens = 10;

}

uint8_t clockFaceRows(ClockFace face) {
  if (face == ClockFace::TALL) {
    return kTallRows;
  }

  return kStaircaseRows;
}

void drawClockFrame(Display& display, uint8_t hour, uint8_t minute, uint8_t minuteShift,
                    uint8_t hourRows, uint8_t minuteRows) {
  const int8_t shift = static_cast<int8_t>(minuteShift);
  const int8_t minuteY = static_cast<int8_t>(Display::kHeight - minuteRows);

  display.clearCanvas();
  drawScaledDigit(display, static_cast<uint8_t>(hour / kTens), kHourTensX, 0, hourRows);
  drawScaledDigit(display, static_cast<uint8_t>(hour % kTens), kHourOnesX, 0, hourRows);
  drawScaledDigit(display, static_cast<uint8_t>(minute / kTens),
                  static_cast<int8_t>(kFaceMinuteX - shift), minuteY, minuteRows);
  drawScaledDigit(display, static_cast<uint8_t>(minute % kTens),
                  static_cast<int8_t>(kMinuteOnesX - shift), minuteY, minuteRows);
}
