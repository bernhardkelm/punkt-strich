#pragma once

#include <cstdint>

/**
 * Signal of one coin on its touch pin: an exponentially smoothed delta above the baseline, a
 * baseline that drifts only while the caller says the coins are idle, the strength as a share of
 * the full touch delta, the peak delta of the running gesture and the dome level.
 */
class CoinSignal {
public:
  /**
   * Binds the signal to its pin, nothing is read until calibrate() or sample().
   * @param {uint8_t} pin - GPIO carrying a touch channel
   */
  explicit CoinSignal(uint8_t pin);

  /**
   * Returns one raw reading of the pin.
   * @returns {float} - raw touch count
   */
  float read() const;

  /**
   * Sets the baseline from averaged raw reads and clears the delta, peak, strength and level.
   * @param {float} base - average raw reading with the finger away
   * @returns {void}
   */
  void calibrate(float base);

  /**
   * Reads the pin, smooths the delta, drifts the baseline when allowed and updates the strength,
   * the gesture peak and the dome level.
   * @param {bool} drift - true lets the baseline follow the reading
   * @param {float} touchShare - full touch delta as a share of the baseline
   * @returns {void}
   */
  void sample(bool drift, float touchShare);

  /**
   * Drops the gesture peak, the next sample starts a new one.
   * @returns {void}
   */
  void resetGestureMax();

  /**
   * Takes the current smoothed reading as the new baseline, used after a step in the ground
   * reference such as plugging or unplugging USB.
   * @returns {void}
   */
  void rebase();

  /**
   * Returns the signal the position estimate uses, see normalisedSignal().
   * @param {float} touchShare - full touch delta as a share of the baseline
   * @returns {float} - normalised signal from 0
   */
  float normalised(float touchShare) const;

  /**
   * Returns the baseline.
   * @returns {float} - reference level in raw counts
   */
  float base() const;

  /**
   * Returns the smoothed delta above the baseline.
   * @returns {float} - delta in raw counts
   */
  float delta() const;

  /**
   * Returns the smoothed delta as a share of the full touch delta.
   * @returns {float} - strength, about 2 for a firm touch on the coin centre
   */
  float strength() const;

  /**
   * Returns the dome level of the last sample.
   * @returns {uint8_t} - level 0 to 3
   */
  uint8_t level() const;

private:
  /** weight of a new reading in the delta smoothing */
  static constexpr float kDeltaWeight = 0.6f;
  /** weight of a new reading in the baseline drift */
  static constexpr float kDriftWeight = 0.002f;

  uint8_t pin_;
  float base_ = 0.0f;
  float delta_ = 0.0f;
  float gestureMax_ = 0.0f;
  float strength_ = 0.0f;
  uint8_t level_ = 0;
};
