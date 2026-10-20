#include "animations/marker.h"

#include "animations/blink.h"
#include "animations/contextGauge.h"

namespace {

constexpr uint32_t kPeriodMs = 4000;
constexpr uint32_t kOnMs = 3500;

}

bool markerLit(uint32_t elapsed) {
  return blinkLitFirst(elapsed, kPeriodMs, kOnMs);
}

uint8_t markerBrightness(uint8_t maxBrightness) {
  return maxBrightness / 2 > 1 ? maxBrightness / 2 : 1;
}

void drawMarker(Display& display, bool lit, uint8_t maxBrightness, uint8_t contextPercent) {
  display.setBrightnessNow(markerBrightness(maxBrightness));
  display.clear();

  if (contextPercent > 0) {
    drawContextGauge(display, contextPercent);
  }

  display.setPixel(Display::kWidth - 1, 0, lit);
  display.show();
}
