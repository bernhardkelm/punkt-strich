#include "animations/IdleGauge.h"

#include "animations/contextGauge.h"

const char* IdleGauge::name() const {
  return "idle-gauge";
}

void IdleGauge::begin(Display& display) {
  percent_ = contextPercent();
  draw(display);
}

void IdleGauge::tick(Display& display, uint32_t) {
  const uint8_t percent = contextPercent();

  if (percent == percent_) {
    return;
  }

  percent_ = percent;
  draw(display);
}

void IdleGauge::draw(Display& display) const {
  display.clear();

  if (percent_ > 0) {
    drawContextGauge(display, percent_);
  }

  display.show();
}
