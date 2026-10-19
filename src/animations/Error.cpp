#include "animations/Error.h"

#include "animations/glitchFrame.h"
#include "animations/skullFrame.h"

#include <Arduino.h>

const char* Error::name() const {
  return "error";
}

void Error::begin(Display& display) {
  startMs_ = millis();
  burst_ = 0;
  draw(display, false);
}

void Error::tick(Display& display, uint32_t nowMs) {
  const uint8_t burst = burstAt((nowMs - startMs_) % kPeriodMs);

  if (burst == burst_) {
    return;
  }

  burst_ = burst;

  if (burst == 0) {
    draw(display, false);
    return;
  }

  uint8_t base[Display::kHeight];
  skullRows(base);
  generateGlitchRows(base, glitchRows_);
  draw(display, true);
}

uint8_t Error::burstAt(uint32_t phase) {
  for (uint8_t i = 0; i < kBurstCount; i++) {
    if (phase >= kBurstStartMs[i] && phase < kBurstEndMs[i]) {
      return static_cast<uint8_t>(i + 1);
    }
  }

  return 0;
}

void Error::draw(Display& display, bool glitched) const {
  if (glitched) {
    drawRows(display, kGlitchBrightness, glitchRows_);
    return;
  }

  uint8_t rows[Display::kHeight];
  skullRows(rows);
  drawRows(display, maxBrightness(), rows);
}
