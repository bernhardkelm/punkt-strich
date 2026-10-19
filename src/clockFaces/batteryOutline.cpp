#include "clockFaces/batteryOutline.h"

#include "clockFaces/columnRun.h"

namespace {

constexpr uint8_t kBodyLeft = kBatteryInsideLeft - 1;
constexpr uint8_t kBodyRight = kBatteryInsideLeft + kBatteryInsideWidth;
constexpr uint8_t kBodyTop = kBatteryInsideTop - 1;
constexpr uint8_t kBodyBottom = kBatteryInsideTop + kBatteryInsideHeight;
constexpr uint8_t kNubX = kBodyRight + 1;
constexpr uint8_t kNubTop = 2;
constexpr uint8_t kNubBottom = 5;

}

Frame batteryOutline() {
  const uint16_t edge = columnRun(kBodyLeft, kBodyRight - kBodyLeft + 1);
  const uint16_t sides = static_cast<uint16_t>(columnRun(kBodyLeft, 1) | columnRun(kBodyRight, 1));
  const uint16_t nub = columnRun(kNubX, 1);
  Frame frame;

  for (uint8_t y = kBodyTop; y <= kBodyBottom; y++) {
    const bool edgeRow = y == kBodyTop || y == kBodyBottom;
    const bool nubRow = y >= kNubTop && y <= kNubBottom;
    frame.rows[y] = edgeRow ? edge : sides;

    if (nubRow) {
      frame.rows[y] = static_cast<uint16_t>(frame.rows[y] | nub);
    }
  }

  return frame;
}
