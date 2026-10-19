#include "clockFaces/minuteBar.h"

#include "clockFaces/columnRun.h"

namespace {

constexpr uint8_t kRowLength = kMinuteBarMax / 2;
constexpr uint8_t kUpperRow = 6;
constexpr uint8_t kLowerRow = 7;
constexpr uint8_t kLastMinute = 59;
constexpr uint16_t kMinutesPerHour = 60;

}

uint8_t minuteBarPixels(uint8_t minute) {
  const uint8_t clamped = minute > kLastMinute ? kLastMinute : minute;
  const uint16_t scaled = static_cast<uint16_t>(clamped * kMinuteBarMax + kMinutesPerHour / 2);
  return static_cast<uint8_t>(scaled / kMinutesPerHour);
}

Frame minuteBarFrame(uint8_t pixels) {
  const uint8_t count = pixels > kMinuteBarMax ? kMinuteBarMax : pixels;
  const uint8_t upper = count > kRowLength ? kRowLength : count;
  Frame frame;
  frame.rows[kUpperRow] = columnRun(0, upper);
  frame.rows[kLowerRow] = columnRun(0, static_cast<uint8_t>(count - upper));
  return frame;
}
