#include "animations/Waiting.h"

#include "animations/contextGauge.h"

#include <Arduino.h>
#include <cmath>

const char* Waiting::name() const {
  return "waiting";
}

void Waiting::begin(Display& display) {
  startMs_ = millis();
  step_ = 0;
  draw(display, step_);
}

void Waiting::tick(Display& display, uint32_t nowMs) {
  const uint32_t step = (nowMs - startMs_) / kStepMs;

  if (step == step_) {
    return;
  }

  step_ = step;
  draw(display, step);
}

void Waiting::draw(Display& display, uint32_t step) const {
  const uint8_t percent = contextPercent();
  const uint8_t rows = usableRows(percent);
  const float phase = static_cast<float>(step);
  const uint8_t x = static_cast<uint8_t>(lroundf((sinf(phase * kXFreq) + 1.0f) * 0.5f * (Display::kWidth - 1)));
  const uint8_t y = static_cast<uint8_t>(lroundf((sinf(phase * kYFreq + kYPhase) + 1.0f) * 0.5f * (rows - 1)));
  const uint8_t brightness = maxBrightness() / 2 > 1 ? maxBrightness() / 2 : 1;

  display.setBrightnessNow(brightness);
  display.clear();

  if (percent > 0) {
    drawContextGauge(display, percent);
  }

  display.setPixel(x, y, true);
  display.show();
}
