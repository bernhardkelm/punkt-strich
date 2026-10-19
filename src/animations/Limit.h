#pragma once

#include "Animation.h"

/** Hourglass with sand drifting from the top half into the bottom half. */
class Limit : public Animation {
public:
  const char* name() const override;
  void begin(Display& display) override;
  void tick(Display& display, uint32_t nowMs) override;

private:
  static constexpr uint32_t kFrameMs = 500;
  static constexpr uint8_t kFrameCount = 8;
  static constexpr uint8_t kLevelCount = 4;

  uint32_t startMs_ = 0;
  uint8_t frame_ = 0;

  /**
   * Returns the pixel mask of one hourglass row for a sand level.
   * @param {uint8_t} y - row index, 0 to 7
   * @param {uint8_t} level - remaining top sand, 3 (full) down to 0 (drained)
   * @returns {uint8_t} - one bit per lit column
   */
  static uint8_t rowMask(uint8_t y, uint8_t level);

  /**
   * Draws one frame of the hourglass.
   * @param {Display&} display - matrix to draw on
   * @param {uint8_t} frame - frame index, 0 to kFrameCount - 1
   * @returns {void}
   */
  void draw(Display& display, uint8_t frame) const;
};
