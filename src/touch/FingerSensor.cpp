#include "touch/FingerSensor.h"

#include "touch/Touch.h"

#include <Arduino.h>

FingerSensor::FingerSensor(uint8_t leftPin, uint8_t rightPin)
    : coins_{CoinSignal(leftPin), CoinSignal(rightPin)},
      column_(static_cast<float>(kSampleMs) / 1000.0f) {
}

void FingerSensor::begin() {
  // shared peripheral, the cycles must match Touch and be set before the baseline is averaged
  touchSetCycles(Touch::kMeasureCycles, Touch::kSleepCycles);
  coins_[0].read();
  coins_[1].read();
  delay(kWarmupMs);
  float sums[2] = {};

  for (uint8_t n = 0; n < kCalibrationReads; n++) {
    for (uint8_t i = 0; i < 2; i++) {
      sums[i] += coins_[i].read();
    }

    delay(kCalibrationDelayMs);
  }

  for (uint8_t i = 0; i < 2; i++) {
    coins_[i].calibrate(sums[i] / kCalibrationReads);
  }

  strength_ = 0.0f;
  column_.reset();
  Serial.printf("punkt: coin baselines left=%.0f right=%.0f\n", coins_[0].base(),
                coins_[1].base());
}

void FingerSensor::setTouchShare(float share) {
  if (share <= 0.0f) {
    return;
  }

  touchShare_ = share;
}

void FingerSensor::sample(bool idle) {
  for (CoinSignal& signal : coins_) {
    signal.sample(idle, touchShare_);
  }

  const float left = coins_[0].strength();
  const float right = coins_[1].strength();
  strength_ = left > right ? left : right;

  if (resting()) {
    column_.reset();

    for (CoinSignal& signal : coins_) {
      signal.resetGestureMax();
    }
  }

  column_.update(coins_[0].normalised(touchShare_), coins_[1].normalised(touchShare_));
}

float FingerSensor::strength() const {
  return strength_;
}

float FingerSensor::strength(Coin coin) const {
  return coinAt(coin).strength();
}

void FingerSensor::rebase() {
  coins_[0].rebase();
  coins_[1].rebase();
  strength_ = 0.0f;
  column_.reset();
}

bool FingerSensor::resting() const {
  return strength_ < kActiveShare * kRestShare;
}

uint8_t FingerSensor::level(Coin coin) const {
  return coinAt(coin).level();
}

float FingerSensor::fingerX() const {
  return column_.value();
}

void FingerSensor::logSample(uint32_t nowMs, uint8_t phase, float radius) const {
  Serial.printf("s,%lu,%.0f,%.0f,%.1f,%.2f,%u,%.1f,%u%u\n", static_cast<unsigned long>(nowMs),
                coins_[0].delta(), coins_[1].delta(), column_.value(), strength_,
                static_cast<unsigned>(phase), radius, static_cast<unsigned>(coins_[0].level()),
                static_cast<unsigned>(coins_[1].level()));
}

void FingerSensor::logIdle(uint32_t nowMs) {
  if ((nowMs - idleLogMs_) < kIdleLogMs) {
    return;
  }

  idleLogMs_ = nowMs;
  Serial.printf("i,%lu,%.0f,%.0f,%.0f,%.0f\n", static_cast<unsigned long>(nowMs),
                coins_[0].delta(), coins_[1].delta(), coins_[0].base(), coins_[1].base());
}

const CoinSignal& FingerSensor::coinAt(Coin coin) const {
  return coins_[static_cast<uint8_t>(coin)];
}
