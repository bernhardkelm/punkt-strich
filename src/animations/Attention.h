#pragma once

#include "Animation.h"

/** Rings blooming out of the centre, a full flash and a dark pause, looping once a second. */
class Attention : public Animation {
public:
  const char* name() const override;
  void begin(Display& display) override;
  void tick(Display& display, uint32_t nowMs) override;

private:
  static constexpr uint32_t kPeriodMs = 1000;
  static constexpr uint32_t kRingMs = 125;
  static constexpr uint32_t kDarkStartMs = 650;
  static constexpr uint8_t kRingCount = 4;

  uint32_t startMs_ = 0;
  uint8_t frame_ = 0;

  /**
   * Maps a point in the loop to a frame index.
   * @param {uint32_t} phase - milliseconds into the period
   * @returns {uint8_t} - 0 to kRingCount - 1 for rings, kRingCount for the dark pause
   */
  static uint8_t frameAt(uint32_t phase);

  /**
   * Draws one frame.
   * @param {Display&} display - matrix to draw on
   * @param {uint8_t} frame - frame index from frameAt()
   * @returns {void}
   */
  void draw(Display& display, uint8_t frame) const;
};
