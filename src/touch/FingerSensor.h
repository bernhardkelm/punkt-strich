#pragma once

#include "touch/Coin.h"
#include "touch/CoinSignal.h"
#include "touch/FingerColumn.h"

#include <cstdint>

/**
 * Samples both coins of strich and turns them into the stronger coin's strength and the filtered
 * finger column. The baselines drift only while the caller reports idle; once both coins fell
 * below half of kActiveShare the slew limit and the One Euro filter restart and both gesture
 * peaks are dropped, so the next finger is located from its own signal.
 */
class FingerSensor {
public:
  /** interval the caller samples at, the One Euro filter is tuned for it */
  static constexpr uint32_t kSampleMs = 10;
  /** strength at which a finger counts as near the coins */
  static constexpr float kActiveShare = 0.13f;

  /**
   * Binds the coins to their pins, nothing is read until begin().
   * @param {uint8_t} leftPin - touch GPIO of the left coin
   * @param {uint8_t} rightPin - touch GPIO of the right coin
   */
  FingerSensor(uint8_t leftPin, uint8_t rightPin);

  /**
   * Applies the measurement cycles, warms both channels up and calibrates both baselines with
   * the finger away. Blocks for about 210 ms.
   * @returns {void}
   */
  void begin();

  /**
   * Sets the full touch delta as a share of the baseline, values at or below 0 are ignored.
   * @param {float} share - touch trigger percent divided by 100
   * @returns {void}
   */
  void setTouchShare(float share);

  /**
   * Samples both coins and updates the strength and the finger column.
   * @param {bool} idle - true lets both baselines drift
   * @returns {void}
   */
  void sample(bool idle);

  /**
   * Rebases both coins on their current readings and drops the column estimate.
   * @returns {void}
   */
  void rebase();

  /**
   * Returns the strength of the stronger coin.
   * @returns {float} - strength of the last sample
   */
  float strength() const;

  /**
   * Returns the strength of one coin.
   * @param {Coin} coin - coin to ask about
   * @returns {float} - strength of the last sample
   */
  float strength(Coin coin) const;

  /**
   * Returns whether both coins are below half of kActiveShare, where the filters restart.
   * @returns {bool} - true while no finger is near either coin
   */
  bool resting() const;

  /**
   * Returns the dome level of one coin.
   * @param {Coin} coin - coin to ask about
   * @returns {uint8_t} - level 0 to 3
   */
  uint8_t level(Coin coin) const;

  /**
   * Returns the filtered finger column.
   * @returns {float} - column 0 to 15
   */
  float fingerX() const;

  /**
   * Prints the CSV line of an active sample: deltas, column, strength, phase, disc radius, levels.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @param {uint8_t} phase - gesture phase of the caller
   * @param {float} radius - swipe disc radius of the caller
   * @returns {void}
   */
  void logSample(uint32_t nowMs, uint8_t phase, float radius) const;

  /**
   * Prints the CSV line of an idle sample, deltas and baselines, at most every kIdleLogMs.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void logIdle(uint32_t nowMs);

private:
  static constexpr uint32_t kWarmupMs = 50;
  static constexpr uint8_t kCalibrationReads = 16;
  static constexpr uint32_t kCalibrationDelayMs = 10;
  /** share of kActiveShare below which the filters restart */
  static constexpr float kRestShare = 0.5f;
  static constexpr uint32_t kIdleLogMs = 500;

  /**
   * Returns the signal of a coin.
   * @param {Coin} coin - coin to look up
   * @returns {const CoinSignal&} - its signal
   */
  const CoinSignal& coinAt(Coin coin) const;

  CoinSignal coins_[2];
  FingerColumn column_;
  float touchShare_ = 0.15f;
  float strength_ = 0.0f;
  uint32_t idleLogMs_ = 0;
};
