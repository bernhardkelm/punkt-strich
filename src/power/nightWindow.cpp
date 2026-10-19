#include "power/nightWindow.h"

namespace {

constexpr uint16_t kMinutesPerDay = 1440;

}

bool insideNightWindow(uint16_t nowMin, uint16_t fromMin, uint16_t toMin) {
  const uint16_t spanMin =
      static_cast<uint16_t>((toMin + kMinutesPerDay - fromMin) % kMinutesPerDay);
  const uint16_t sinceFrom =
      static_cast<uint16_t>((nowMin + kMinutesPerDay - fromMin) % kMinutesPerDay);
  return sinceFrom < spanMin;
}
