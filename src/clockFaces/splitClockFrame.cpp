#include "clockFaces/splitClockFrame.h"

#include "clockFaces/digitPairs.h"
#include "clockFaces/frameUnion.h"
#include "clockFaces/minuteBar.h"
#include "clockFaces/minuteSquares.h"

namespace {

constexpr uint8_t kTens = 10;
constexpr int8_t kStaggerMinutesX = 1;
constexpr int8_t kStaggerMinutesY = 4;

/**
 * Returns the minute part of the split clock in a style.
 * @param {SplitClock} style - split clock style
 * @param {uint8_t} minute - 0 to 59
 * @returns {Frame} - frame with the minute pixels set
 */
Frame minutesFrame(SplitClock style, uint8_t minute) {
  switch (style) {
    case SplitClock::SQUARES:
      return minuteSquaresFrame(static_cast<uint8_t>(minute / kTens),
                                static_cast<uint8_t>(minute % kTens));
    case SplitClock::STAGGER:
      return digitPairFrame(minute, kStaggerMinutesX, kStaggerMinutesY);
    case SplitClock::BAR:
      return minuteBarFrame(minuteBarPixels(minute));
    case SplitClock::DIGITS:
    default:
      return smallPairFrame(minute, 0, kSmallMinutesY);
  }
}

}

Frame splitHoursFrame(uint8_t hour) {
  return digitPairFrame(hour, 0, 0);
}

Frame splitClockFrame(SplitClock style, uint8_t hour, uint8_t minute) {
  return frameUnion(splitHoursFrame(hour), minutesFrame(style, minute));
}
