#pragma once

#include "Animation.h"

/** Single pixel drifting slowly along a Lissajous-like path, quieter than attention. */
class Waiting : public Animation {
public:
  const char* name() const override;
  void begin(Display& display) override;
  void tick(Display& display, uint32_t nowMs) override;
  void applyBrightness(Display& display, uint32_t fadeMs) override;

private:
  static constexpr uint32_t kStepMs = 400;
  static constexpr float kXFreq = 0.29f;
  static constexpr float kYFreq = 0.37f;
  static constexpr float kYPhase = 1.7f;

  uint32_t startMs_ = 0;
  uint32_t step_ = 0;

  /**
   * Returns the level the drifting pixel runs at, half the ceiling and at least 1.
   * @returns {uint8_t} - 1 to 7
   */
  uint8_t level() const;

  /**
   * Draws the drifting pixel and the context gauge for a step.
   * @param {Display&} display - matrix to draw on
   * @param {uint32_t} step - step index, one every kStepMs
   * @returns {void}
   */
  void draw(Display& display, uint32_t step) const;
};
