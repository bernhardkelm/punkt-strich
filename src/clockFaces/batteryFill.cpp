#include "clockFaces/batteryFill.h"

#include "clockFaces/batteryOutline.h"
#include "clockFaces/columnRun.h"

namespace {

constexpr uint8_t kFullPercent = 100;

}

Frame batteryFill(uint8_t percent) {
  const uint8_t clamped = percent > kFullPercent ? kFullPercent : percent;
  const uint16_t scaled = static_cast<uint16_t>(clamped * kBatteryInsideWidth + kFullPercent / 2);
  const uint16_t bits = columnRun(kBatteryInsideLeft, static_cast<uint8_t>(scaled / kFullPercent));
  Frame frame;

  for (uint8_t row = 0; row < kBatteryInsideHeight; row++) {
    frame.rows[kBatteryInsideTop + row] = bits;
  }

  return frame;
}
