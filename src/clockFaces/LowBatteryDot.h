#pragma once

#include <cstdint>

/**
 * Low-battery dot a clock face draws over its picture in the bottom row's column 7, 500 ms lit and
 * 500 ms dark, the rhythm restarting whenever the dot turns on. The face asks update() on every
 * tick and redraws when it reports a change.
 */
class LowBatteryDot {
public:
  /** Column of the dot, on the canvas for a full face and in the viewport for the split clock */
  static constexpr int8_t kX = 7;
  static constexpr int8_t kY = 7;

  /**
   * Turns the dot on or off, turning it on restarts its rhythm. The next update() picks it up.
   * @param {bool} on - true while the battery runs low
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void set(bool on, uint32_t nowMs);

  /**
   * Moves the dot to its rhythm at a moment.
   * @param {uint32_t} nowMs - millis() to evaluate
   * @returns {bool} - true when active() or lit() changed, so the face redraws
   */
  bool update(uint32_t nowMs);

  /**
   * Returns whether the dot overrides its pixel, lit or dark, as of the last update().
   * @returns {bool} - true while the dot is on
   */
  bool active() const;

  /**
   * Returns whether the dot is lit as of the last update().
   * @returns {bool} - true in the lit half of the rhythm, false while the dot is off
   */
  bool lit() const;

private:
  static constexpr uint32_t kPeriodMs = 1000;
  static constexpr uint32_t kOnMs = 500;

  uint32_t startMs_ = 0;
  bool on_ = false;
  bool active_ = false;
  bool lit_ = false;
};
