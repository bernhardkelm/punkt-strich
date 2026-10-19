#include "power/batteryEstimate.h"

#include <cstddef>

namespace {

/** One point of the discharge curve. */
struct CurvePoint {
  int mv;
  uint8_t percent;
};

// ordered from full to empty, VBUS is about the cell voltage less 0.85 V
constexpr CurvePoint kCurve[] = {{3300, 100}, {3150, 85}, {3050, 70}, {2950, 55},
                                 {2850, 35},  {2750, 15}, {2600, 0}};
constexpr size_t kCurveCount = sizeof(kCurve) / sizeof(kCurve[0]);

}

uint8_t batteryPercent(int vbusMv) {
  if (vbusMv >= kCurve[0].mv) {
    return kCurve[0].percent;
  }

  for (size_t i = 1; i < kCurveCount; i++) {
    const CurvePoint& high = kCurve[i - 1];
    const CurvePoint& low = kCurve[i];

    if (vbusMv < low.mv) {
      continue;
    }

    const int span = high.mv - low.mv;
    const int rise = high.percent - low.percent;
    const int above = vbusMv - low.mv;
    return static_cast<uint8_t>(low.percent + (above * rise + span / 2) / span);
  }

  return kCurve[kCurveCount - 1].percent;
}
