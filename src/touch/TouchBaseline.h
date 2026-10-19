#pragma once

#include <cstdint>

/** Holds the untouched reference level of the electrode and compares raw reads against it. */
class TouchBaseline {
public:
  /**
   * Sets the reference level, used after the calibration in begin().
   * @param {uint32_t} value - averaged raw read of the untouched electrode
   * @returns {void}
   */
  void reset(uint32_t value);

  /**
   * Returns the current reference level.
   * @returns {uint32_t} - reference level in raw counts
   */
  uint32_t value() const;

  /**
   * Moves the reference level one exponential step towards a raw read.
   * @param {uint32_t} raw - raw read of the untouched electrode
   * @param {uint32_t} divisor - larger values slow the tracking down, 0 disables it
   * @returns {void}
   */
  void drift(uint32_t raw, uint32_t divisor);

  /**
   * Returns whether a raw read reaches a percentage of the reference level.
   * @param {uint32_t} raw - raw read to compare
   * @param {uint32_t} percent - percentage of the reference level, 125 means 1.25 times
   * @returns {bool} - true when raw is at or above that percentage
   */
  bool atOrAbovePercent(uint32_t raw, uint32_t percent) const;

  /**
   * Returns whether a raw read fell back to a percentage of the reference level.
   * @param {uint32_t} raw - raw read to compare
   * @param {uint32_t} percent - percentage of the reference level, 110 means 1.10 times
   * @returns {bool} - true when raw is at or below that percentage
   */
  bool atOrBelowPercent(uint32_t raw, uint32_t percent) const;

private:
  uint32_t value_ = 0;
};
