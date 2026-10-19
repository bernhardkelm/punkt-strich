#include "BrightnessRamp.h"

void BrightnessRamp::rampTo(uint8_t level, uint32_t durationMs, uint32_t nowMs) {
  const uint8_t target = level > kMaxLevel ? kMaxLevel : level;

  if (target == target_ && durationMs != 0) {
    return;
  }

  from_ = currentAt(nowMs);
  target_ = target;
  startMs_ = nowMs;
  durationMs_ = durationMs;
}

bool BrightnessRamp::tick(uint32_t nowMs, uint8_t& level) {
  level = currentAt(nowMs);

  if (level == written_) {
    return false;
  }

  written_ = level;
  return true;
}

void BrightnessRamp::reset() {
  written_ = kUnset;
}

uint8_t BrightnessRamp::currentAt(uint32_t nowMs) const {
  const uint32_t elapsed = nowMs - startMs_;

  if (durationMs_ == 0 || elapsed >= durationMs_) {
    return target_;
  }

  const int32_t delta = static_cast<int32_t>(target_) - static_cast<int32_t>(from_);
  const int32_t duration = static_cast<int32_t>(durationMs_);
  const int32_t rounding = delta >= 0 ? duration : -duration;
  const int32_t offset = (2 * delta * static_cast<int32_t>(elapsed) + rounding) / (2 * duration);
  return static_cast<uint8_t>(static_cast<int32_t>(from_) + offset);
}
