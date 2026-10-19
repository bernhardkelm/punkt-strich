#include "touch/CoinSignal.h"

#include "touch/domeLevels.h"
#include "touch/fingerPosition.h"

#include <Arduino.h>

CoinSignal::CoinSignal(uint8_t pin) : pin_(pin) {
}

float CoinSignal::read() const {
  return static_cast<float>(touchRead(pin_));
}

void CoinSignal::calibrate(float base) {
  base_ = base;
  delta_ = 0.0f;
  gestureMax_ = 0.0f;
  strength_ = 0.0f;
  level_ = 0;
}

void CoinSignal::sample(bool drift, float touchShare) {
  const float raw = read();
  delta_ += (raw - base_ - delta_) * kDeltaWeight;

  if (drift) {
    base_ += (raw - base_) * kDriftWeight;
  }

  strength_ = delta_ / (base_ * touchShare);
  gestureMax_ = delta_ > gestureMax_ ? delta_ : gestureMax_;
  level_ = nextDomeLevel(level_, strength_);
}

void CoinSignal::resetGestureMax() {
  gestureMax_ = 0.0f;
}

void CoinSignal::rebase() {
  base_ += delta_;
  delta_ = 0.0f;
  strength_ = 0.0f;
  gestureMax_ = 0.0f;
  level_ = 0;
}

float CoinSignal::normalised(float touchShare) const {
  return normalisedSignal(delta_, gestureMax_, base_ * touchShare);
}

float CoinSignal::base() const {
  return base_;
}

float CoinSignal::delta() const {
  return delta_;
}

float CoinSignal::strength() const {
  return strength_;
}

uint8_t CoinSignal::level() const {
  return level_;
}
