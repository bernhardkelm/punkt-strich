#include "clockFaces/LowBatteryDot.h"

#include "animations/blink.h"

void LowBatteryDot::set(bool on, uint32_t nowMs) {
  if (on && !on_) {
    startMs_ = nowMs;
  }

  on_ = on;
}

bool LowBatteryDot::update(uint32_t nowMs) {
  const bool lit = on_ && blinkLitFirst(nowMs - startMs_, kPeriodMs, kOnMs);
  const bool changed = on_ != active_ || lit != lit_;
  active_ = on_;
  lit_ = lit;
  return changed;
}

bool LowBatteryDot::active() const {
  return active_;
}

bool LowBatteryDot::lit() const {
  return lit_;
}
