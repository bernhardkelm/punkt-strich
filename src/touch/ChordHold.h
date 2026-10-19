#pragma once

#include <cstdint>

/**
 * Two-finger hold: starts once both coins reach kEnterStrength in the same sample, which a single
 * finger in the middle cannot (it reaches about 1.1 and 0.9), and ends once either coin falls
 * below kExitShare of it.
 */
class ChordHold {
public:
  /**
   * Starts or ends the hold from one sample of both coin strengths.
   * @param {float} leftStrength - strength of the left coin
   * @param {float} rightStrength - strength of the right coin
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void update(float leftStrength, float rightStrength, uint32_t nowMs);

  /**
   * Returns whether the hold is running.
   * @returns {bool} - true from the sample both coins reached kEnterStrength until it ended
   */
  bool active() const;

  /**
   * Returns how long the running hold lasts.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {uint32_t} - milliseconds since it started, 0 while no hold runs
   */
  uint32_t heldMs(uint32_t nowMs) const;

  /**
   * Ends a running hold without waiting for the coins to fall.
   * @returns {void}
   */
  void reset();

private:
  /** each coin and both together must reach these, one finger in the middle stays below */
  static constexpr float kEnterStrength = 1.0f;
  static constexpr float kEnterSum = 2.4f;
  /** share of the entry levels the coins must stay above to keep the hold running */
  static constexpr float kExitShare = 0.7f;

  uint32_t startMs_ = 0;
  bool active_ = false;
};
