#pragma once

#include <cstdint>

/** Linear brightness ramps between the 16 hardware levels, one write per level change. */
class BrightnessRamp {
public:
  static constexpr uint8_t kMaxLevel = 15;

  /**
   * Starts a ramp from the current level to a target.
   * @param {uint8_t} level - target level, clamped to kMaxLevel
   * @param {uint32_t} durationMs - ramp length, 0 applies immediately
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void rampTo(uint8_t level, uint32_t durationMs, uint32_t nowMs);

  /**
   * Advances the ramp and reports the level to write.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @param {uint8_t&} level - receives the level to write
   * @returns {bool} - true when the level differs from the last one returned
   */
  bool tick(uint32_t nowMs, uint8_t& level);

  /** Forgets the last returned level so the next tick reports a change. */
  void reset();

private:
  static constexpr uint8_t kUnset = 0xFF;

  uint8_t from_ = kMaxLevel;
  uint8_t target_ = kMaxLevel;
  uint32_t startMs_ = 0;
  uint32_t durationMs_ = 0;
  uint8_t written_ = kUnset;

  /**
   * Interpolates the ramp, rounded to the nearest level.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {uint8_t} - current level
   */
  uint8_t currentAt(uint32_t nowMs) const;
};
