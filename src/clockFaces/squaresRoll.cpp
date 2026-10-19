#include "clockFaces/squaresRoll.h"

namespace {

constexpr uint16_t kMinutesPerHour = 60;
constexpr uint16_t kTens = 10;

}

uint16_t squaresRollStep(uint16_t shown, uint16_t target) {
  const bool sameHour = shown / kMinutesPerHour == target / kMinutesPerHour;
  const uint16_t minute = shown % kMinutesPerHour;
  const uint16_t targetMinute = target % kMinutesPerHour;

  if (sameHour && minute / kTens == targetMinute / kTens) {
    return target;
  }

  if (minute % kTens > 0) {
    return static_cast<uint16_t>(shown - 1);
  }

  if (!sameHour && minute > 0) {
    return static_cast<uint16_t>(shown - kTens);
  }

  return target;
}
