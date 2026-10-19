#include "touch/Touch.h"

#include <Arduino.h>

void Touch::snapDown() {
  if (touched_ || !baseline_.atOrBelowPercent(raw_, kSnapBelowPercent)) {
    lowSamples_ = 0;
    return;
  }

  lowSamples_++;

  if (lowSamples_ < kDebounceSamples) {
    return;
  }

  lowSamples_ = 0;
  const uint32_t target = snapTarget();

  if (target >= baseline_.value()) {
    return;
  }

  Serial.printf("punkt: %s baseline %u -> %u, reading dropped\n", name_, baseline_.value(),
                target);
  baseline_.reset(target);
}

uint32_t Touch::snapTarget() const {
  // a step past the release threshold would read as a touch the recovered level can never release
  const uint64_t limit =
      (static_cast<uint64_t>(baseline_.value()) * 100 + releasePercent_ - 1) / releasePercent_;

  if (raw_ > limit) {
    return raw_;
  }

  return static_cast<uint32_t>(limit);
}

void Touch::trackDrift(uint32_t nowMs) {
  if (pending_ != 0 || (touched_ && !stuckTouch(nowMs))) {
    driftMs_ = nowMs;
    return;
  }

  if (hoverHoldsDrift(nowMs)) {
    driftMs_ = nowMs;
    return;
  }

  if ((nowMs - driftMs_) < kDriftIntervalMs) {
    return;
  }

  driftMs_ = nowMs;
  baseline_.drift(raw_, kDriftDivisor);
}

bool Touch::hoverHoldsDrift(uint32_t nowMs) {
  if (proximity_.level() == 0) {
    hovering_ = false;
    return false;
  }

  if (!hovering_) {
    hovering_ = true;
    hoverMs_ = nowMs;
  }

  return (nowMs - hoverMs_) < kHoverHoldMs;
}

bool Touch::stuckTouch(uint32_t nowMs) {
  if ((nowMs - pressedMs_) < kStuckMs) {
    return false;
  }

  if (!stuck_) {
    stuck_ = true;
    Serial.printf("punkt: %s stuck, tracking the baseline again\n", name_);
  }

  return true;
}
