#pragma once

#include "Animation.h"

#include <cstdint>

/** Fills the whole canvas bottom-up while the BOOT button is held towards the factory reset. */
class ResetFill : public Animation {
public:
  const char* name() const override;
  void begin(Display& display) override;
  void tick(Display& display, uint32_t nowMs) override;

  /**
   * Sets how many pixels are lit, redrawn on the next tick when it changed.
   * @param {uint8_t} pixels - 0 to kCanvasWidth * kHeight
   * @returns {void}
   */
  void setProgress(uint8_t pixels);

private:
  /**
   * Draws the fill from the bottom row upward, left to right within a row.
   * @param {Display&} display - matrix to draw on
   * @returns {void}
   */
  void draw(Display& display);

  uint8_t pixels_ = 0;
  uint8_t drawn_ = 0;
};
