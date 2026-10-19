#pragma once

#include "Animation.h"

/** Rings collapsing from the border into the centre in a short loop. */
class Compacting : public Animation {
public:
  const char* name() const override;
  void begin(Display& display) override;
  void tick(Display& display, uint32_t nowMs) override;

private:
  static constexpr uint32_t kFrameMs = 200;
  static constexpr uint8_t kFrameCount = 6;

  uint32_t startMs_ = 0;
  uint8_t frame_ = 0;

  /**
   * Draws one frame of the collapsing rings.
   * @param {Display&} display - matrix to draw on
   * @param {uint8_t} frame - frame index, 0 to kFrameCount - 1
   * @returns {void}
   */
  void draw(Display& display, uint8_t frame) const;
};
