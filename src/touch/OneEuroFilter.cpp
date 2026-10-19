#include "touch/OneEuroFilter.h"

#include <cmath>

namespace {

constexpr double kPi = 3.14159265358979323846;

}

OneEuroFilter::OneEuroFilter(float minCutoffHz, float beta, float speedCutoffHz, float sampleS)
    : minCutoffHz_(minCutoffHz), beta_(beta), speedCutoffHz_(speedCutoffHz), sampleS_(sampleS) {
}

void OneEuroFilter::reset() {
  primed_ = false;
}

float OneEuroFilter::filter(float raw) {
  if (!primed_) {
    primed_ = true;
    smooth_ = raw;
    lastRaw_ = raw;
    speed_ = 0.0f;
    return smooth_;
  }

  const float rawSpeed = (raw - lastRaw_) / sampleS_;
  lastRaw_ = raw;
  speed_ += (rawSpeed - speed_) * alphaFor(speedCutoffHz_);
  smooth_ += (raw - smooth_) * alphaFor(minCutoffHz_ + beta_ * std::fabs(speed_));
  return smooth_;
}

float OneEuroFilter::alphaFor(float cutoffHz) const {
  const float tau = static_cast<float>(1.0f / (2.0f * kPi * cutoffHz));
  return 1.0f / (1.0f + tau / sampleS_);
}
