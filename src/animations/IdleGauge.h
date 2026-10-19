#pragma once

#include "Animation.h"

/** Dark viewport with only the context gauge on the bottom row, the calm face of idle on strich. */
class IdleGauge : public Animation {
public:
  const char* name() const override;
  void begin(Display& display) override;
  void tick(Display& display, uint32_t nowMs) override;

private:
  uint8_t percent_ = 0;

  /**
   * Clears the viewport and draws the context gauge at the fill last read.
   * @param {Display&} display - matrix to draw on
   * @returns {void}
   */
  void draw(Display& display) const;
};
