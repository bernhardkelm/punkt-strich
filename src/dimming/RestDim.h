#pragma once

#include <cstdint>

class Settings;
class StateMachine;

/** Dims the clock face once it has rested without activity for dimTimeoutMin. */
class RestDim {
public:
  /** Length of the ramp into and out of the rest dim. */
  static constexpr uint32_t kRampMs = 2000;

  /**
   * Evaluates whether the clock counts as resting right now and logs a flip, cheap enough to call
   * on every loop pass without an interval of its own.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @param {const Settings&} settings - dimTimeoutMin, 0 disables the dim
   * @param {const StateMachine&} state - clock view and since when it has rested
   * @returns {bool} - true when the resting flag flipped
   */
  bool tick(uint32_t nowMs, const Settings& settings, const StateMachine& state);

  /**
   * Returns whether the clock currently counts as resting.
   * @returns {bool} - true while the rest dim applies
   */
  bool resting() const;

private:
  /** Longest a nowMs sample may lag a timestamp taken after it, more counts as elapsed time */
  static constexpr uint32_t kSampleLagMaxMs = 1000;

  /**
   * Computes whether the clock counts as resting right now.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @param {const Settings&} settings - dimTimeoutMin, 0 disables the dim
   * @param {const StateMachine&} state - clock view and since when it has rested
   * @returns {bool} - true when dimTimeoutMin minutes passed since restingSinceMs()
   */
  bool evaluate(uint32_t nowMs, const Settings& settings, const StateMachine& state) const;

  bool resting_ = false;
};
