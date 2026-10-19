#pragma once

#include <cstdint>

/**
 * Hover level of the capacitive electrode, 0 for nothing near the pad up to 3 for contact range.
 * The raw reading is smoothed with an exponential average so a single noisy sample cannot move a
 * level, and compared against three entry thresholds derived from the trigger delta: level 3 sits
 * just below the trigger, level 1 on the noise floor of the calibration, level 2 on their
 * geometric middle because the capacitance rises roughly with 1 over the distance. Every level is
 * left again at kReleaseSharePercent of its entry threshold, so a reading resting on a threshold
 * cannot flicker between two levels.
 */
class TouchProximity {
public:
  /**
   * Seeds the smoothed reading with a raw read and drops the level to 0, used after a calibration.
   * @param {uint32_t} raw - raw read of the untouched electrode
   * @returns {void}
   */
  void reset(uint32_t raw);

  /**
   * Feeds one sample into the smoothing and returns the level it lands on.
   * @param {uint32_t} raw - raw read of the electrode
   * @param {uint32_t} baseline - untouched reference level in raw counts
   * @param {uint32_t} triggerDelta - delta above the baseline that counts as a touch
   * @param {uint32_t} spread - max minus min of the readings the calibration averaged
   * @param {uint8_t} hoverPercent - level 1 as percent of the trigger delta, 0 turns the levels off
   * @returns {uint8_t} - hover level 0 to 3
   */
  uint8_t update(uint32_t raw, uint32_t baseline, uint32_t triggerDelta, uint32_t spread,
                 uint8_t hoverPercent);

  /**
   * Returns the level the last update() landed on.
   * @returns {uint8_t} - hover level 0 to 3
   */
  uint8_t level() const;

private:
  static constexpr uint8_t kLevels = 3;
  /** fixed point shift of the smoothed reading, keeps the step below one raw count alive */
  static constexpr uint32_t kFixedShift = 4;
  /** weight of a new sample in the exponential average, 2 means one quarter */
  static constexpr uint32_t kSmoothingShift = 2;
  /** entry threshold of level 3 as a share of the trigger delta */
  static constexpr uint32_t kTopSharePercent = 80;
  /** a level is left once the delta falls below this share of its entry threshold */
  static constexpr uint32_t kReleaseSharePercent = 70;
  /** how many calibration spreads level 1 keeps above the noise floor */
  static constexpr uint32_t kSpreadFactor = 2;

  /** entry thresholds of the levels 1 to 3 in raw counts above the baseline */
  struct Thresholds {
    uint32_t entry[kLevels] = {};
  };

  /**
   * Moves the smoothed reading one exponential step towards a raw read.
   * @param {uint32_t} raw - raw read of the electrode
   * @returns {void}
   */
  void smooth(uint32_t raw);

  /**
   * Returns how far the smoothed reading sits above the baseline.
   * @param {uint32_t} baseline - untouched reference level in raw counts
   * @returns {uint32_t} - delta in raw counts, 0 while the reading is at or below the baseline
   */
  uint32_t smoothedDelta(uint32_t baseline) const;

  /**
   * Derives the three entry thresholds from the trigger delta, the noise floor and the setting.
   * @param {uint32_t} triggerDelta - delta above the baseline that counts as a touch
   * @param {uint32_t} spread - max minus min of the readings the calibration averaged
   * @param {uint8_t} hoverPercent - level 1 as percent of the trigger delta, 0 turns the levels off
   * @returns {Thresholds} - entry thresholds, all zero while the levels are off
   */
  Thresholds thresholdsFor(uint32_t triggerDelta, uint32_t spread, uint8_t hoverPercent) const;

  /**
   * Returns the highest level whose entry threshold the delta reaches.
   * @param {uint32_t} delta - smoothed delta above the baseline in raw counts
   * @param {Thresholds} limits - entry thresholds of the levels 1 to 3
   * @returns {uint8_t} - level 0 to 3
   */
  uint8_t highestEntered(uint32_t delta, const Thresholds& limits) const;

  /**
   * Returns the highest level at or below the current one the delta still holds, so a level is
   * only left once the delta falls below kReleaseSharePercent of its entry threshold.
   * @param {uint32_t} delta - smoothed delta above the baseline in raw counts
   * @param {Thresholds} limits - entry thresholds of the levels 1 to 3
   * @returns {uint8_t} - level 0 to 3
   */
  uint8_t highestHeld(uint32_t delta, const Thresholds& limits) const;

  /** smoothed raw reading, shifted left by kFixedShift */
  uint32_t smoothed_ = 0;
  uint8_t level_ = 0;
};
