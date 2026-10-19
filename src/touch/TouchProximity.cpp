#include "touch/TouchProximity.h"

namespace {

constexpr uint32_t kPercentScale = 100;

/**
 * Returns a percentage of a raw count, widened so large readings cannot overflow the product.
 * @param {uint32_t} value - raw counts
 * @param {uint32_t} percent - percentage to take
 * @returns {uint32_t} - value times percent divided by 100
 */
uint32_t share(uint32_t value, uint32_t percent) {
  return static_cast<uint32_t>(static_cast<uint64_t>(value) * percent / kPercentScale);
}

/**
 * Returns the integer square root of a 64 bit value, bit by bit so no floating point is needed.
 * @param {uint64_t} value - radicand
 * @returns {uint32_t} - largest integer whose square stays at or below value
 */
uint32_t integerSqrt(uint64_t value) {
  uint64_t rest = value;
  uint64_t root = 0;
  uint64_t bit = uint64_t{1} << 62;

  while (bit > rest) {
    bit >>= 2;
  }

  while (bit != 0) {
    if (rest >= root + bit) {
      rest -= root + bit;
      root = (root >> 1) + bit;
    } else {
      root >>= 1;
    }

    bit >>= 2;
  }

  return static_cast<uint32_t>(root);
}

}

void TouchProximity::reset(uint32_t raw) {
  smoothed_ = raw << kFixedShift;
  level_ = 0;
}

uint8_t TouchProximity::update(uint32_t raw, uint32_t baseline, uint32_t triggerDelta,
                               uint32_t spread, uint8_t hoverPercent) {
  smooth(raw);

  const Thresholds limits = thresholdsFor(triggerDelta, spread, hoverPercent);

  // the smoothing stays running while the levels are off, so a later sample starts settled
  if (limits.entry[kLevels - 1] == 0) {
    level_ = 0;
    return level_;
  }

  const uint32_t delta = smoothedDelta(baseline);
  const uint8_t entered = highestEntered(delta, limits);
  const uint8_t held = highestHeld(delta, limits);
  level_ = entered > held ? entered : held;
  return level_;
}

uint8_t TouchProximity::level() const {
  return level_;
}

void TouchProximity::smooth(uint32_t raw) {
  const uint32_t target = raw << kFixedShift;

  if (target > smoothed_) {
    smoothed_ += (target - smoothed_) >> kSmoothingShift;
    return;
  }

  smoothed_ -= (smoothed_ - target) >> kSmoothingShift;
}

uint32_t TouchProximity::smoothedDelta(uint32_t baseline) const {
  const uint32_t reading = smoothed_ >> kFixedShift;

  if (reading <= baseline) {
    return 0;
  }

  return reading - baseline;
}

TouchProximity::Thresholds TouchProximity::thresholdsFor(uint32_t triggerDelta, uint32_t spread,
                                                         uint8_t hoverPercent) const {
  Thresholds limits;

  if (hoverPercent == 0) {
    return limits;
  }

  const uint32_t top = share(triggerDelta, kTopSharePercent);

  if (top == 0) {
    return limits;
  }

  uint32_t low = share(triggerDelta, hoverPercent);
  const uint32_t noise = spread * kSpreadFactor;

  if (low < noise) {
    low = noise;
  }

  if (low == 0) {
    low = 1;
  }

  if (low > top) {
    low = top;
  }

  limits.entry[0] = low;
  limits.entry[1] = integerSqrt(static_cast<uint64_t>(low) * top);
  limits.entry[2] = top;
  return limits;
}

uint8_t TouchProximity::highestEntered(uint32_t delta, const Thresholds& limits) const {
  for (uint8_t n = kLevels; n > 0; n--) {
    if (delta >= limits.entry[n - 1]) {
      return n;
    }
  }

  return 0;
}

uint8_t TouchProximity::highestHeld(uint32_t delta, const Thresholds& limits) const {
  for (uint8_t n = level_; n > 0; n--) {
    if (delta >= share(limits.entry[n - 1], kReleaseSharePercent)) {
      return n;
    }
  }

  return 0;
}
