#pragma once

#include "Animation.h"

/** A skull with three short glitch bursts per loop. */
class Error : public Animation {
public:
  const char* name() const override;
  void begin(Display& display) override;
  void tick(Display& display, uint32_t nowMs) override;

private:
  static constexpr uint32_t kPeriodMs = 1600;
  static constexpr uint8_t kBurstCount = 3;
  static constexpr uint32_t kBurstStartMs[kBurstCount] = {1000, 1200, 1400};
  static constexpr uint32_t kBurstEndMs[kBurstCount] = {1120, 1320, 1520};

  uint32_t startMs_ = 0;
  uint8_t burst_ = 0;
  uint8_t glitchRows_[Display::kHeight] = {};

  /**
   * Maps a point in the loop to a burst index.
   * @param {uint32_t} phase - milliseconds into the period
   * @returns {uint8_t} - 0 outside a burst, 1 to kBurstCount inside one
   */
  static uint8_t burstAt(uint32_t phase);

  /**
   * Draws the plain skull or the current glitch frame.
   * @param {Display&} display - matrix to draw on
   * @param {bool} glitched - true to draw the stored glitch frame
   * @returns {void}
   */
  void draw(Display& display, bool glitched) const;
};
