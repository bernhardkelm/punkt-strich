#include "touch/FingerColumn.h"

#include "touch/fingerPosition.h"

#include <algorithm>

FingerColumn::FingerColumn(float sampleS)
    : filter_(kMinCutoffHz, kBeta, kSpeedCutoffHz, sampleS) {
}

void FingerColumn::reset() {
  slewPrimed_ = false;
  filter_.reset();
}

float FingerColumn::update(float left, float right) {
  const float raw = left + right > 0.0f ? limit(columnFrom(left, right)) : value_;
  value_ = filter_.filter(raw);
  return value_;
}

float FingerColumn::value() const {
  return value_;
}

float FingerColumn::limit(float x) {
  if (!slewPrimed_) {
    slewPrimed_ = true;
    slewX_ = x;
    return slewX_;
  }

  slewX_ += std::clamp(x - slewX_, -kMaxStep, kMaxStep);
  return slewX_;
}
