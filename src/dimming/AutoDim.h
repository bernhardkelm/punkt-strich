#pragma once

#include <cstdint>

class Settings;

/** Maps the sun's elevation onto the brightness range between the day and the night level. */
class AutoDim {
public:
  /** Elevation at and below which the night level applies, end of civil twilight. */
  static constexpr float kNightElevationDeg = -6.0f;
  /** Elevation at and above which the day level applies. */
  static constexpr float kDayElevationDeg = 10.0f;
  /** Interval between two evaluations in tick(). */
  static constexpr uint32_t kCheckIntervalMs = 30000;

  /**
   * Recomputes the level right away, regardless of the interval, and restarts it.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @param {const Settings&} settings - autoDim flag, day and night levels, coordinates
   * @returns {uint8_t} - brightness ceiling to apply, 0 to 15
   */
  uint8_t refresh(uint32_t nowMs, const Settings& settings);

  /**
   * Evaluates the level at most every kCheckIntervalMs, the first call evaluates at once.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @param {const Settings&} settings - autoDim flag, day and night levels, coordinates
   * @param {uint8_t&} level - receives the ceiling to apply, untouched while it did not change
   * @returns {bool} - true when the level differs from the last one reported
   */
  bool tick(uint32_t nowMs, const Settings& settings, uint8_t& level);

  /**
   * Returns the level last evaluated by refresh() or tick().
   * @returns {uint8_t} - brightness ceiling, 0 to 15
   */
  uint8_t level() const;

private:
  static constexpr uint8_t kUnset = 0xFF;

  uint8_t level_ = kUnset;
  uint32_t checkedMs_ = 0;
  bool checked_ = false;

  /**
   * Computes the ceiling for the current moment and logs a changed level, the day level while
   * the automatic dimming is off or the clock is not synced yet.
   * @param {const Settings&} settings - autoDim flag, day and night levels, coordinates
   * @returns {uint8_t} - brightness ceiling, 0 to 15
   */
  uint8_t evaluate(const Settings& settings);
};
