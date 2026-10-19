#include "touch/TouchBaseline.h"

namespace {

constexpr uint32_t kPercentScale = 100;

/** Returns the step for a distance, at least one count so integer division cannot stall the tracking. */
uint32_t driftStep(uint32_t distance, uint32_t divisor) {
  const uint32_t step = distance / divisor;

  if (step == 0) {
    return 1;
  }

  return step;
}

}

void TouchBaseline::reset(uint32_t value) {
  value_ = value;
}

uint32_t TouchBaseline::value() const {
  return value_;
}

void TouchBaseline::drift(uint32_t raw, uint32_t divisor) {
  if (divisor == 0 || raw == value_) {
    return;
  }

  if (raw > value_) {
    value_ += driftStep(raw - value_, divisor);
    return;
  }

  value_ -= driftStep(value_ - raw, divisor);
}

bool TouchBaseline::atOrAbovePercent(uint32_t raw, uint32_t percent) const {
  return static_cast<uint64_t>(raw) * kPercentScale >= static_cast<uint64_t>(value_) * percent;
}

bool TouchBaseline::atOrBelowPercent(uint32_t raw, uint32_t percent) const {
  return static_cast<uint64_t>(raw) * kPercentScale <= static_cast<uint64_t>(value_) * percent;
}
