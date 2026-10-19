#pragma once

#include "Animation.h"

/** A 2x2 block sweeping across the centre rows with a trail behind it. */
class Knight : public Animation {
public:
  const char* name() const override;
  void begin(Display& display) override;
  void tick(Display& display, uint32_t nowMs) override;

private:
  static constexpr uint32_t kCycleMs = 1400;
  static constexpr uint8_t kBlockSize = 2;
  static constexpr uint8_t kTopRow = 3;
  static constexpr uint8_t kMaxLeft = Display::kWidth - kBlockSize;

  uint32_t startMs_ = 0;
  int8_t left_ = -1;
  bool movingRight_ = true;

  /**
   * Draws the block at a left edge plus the trailing column behind it.
   * @param {Display&} display - matrix to draw on
   * @param {int8_t} left - left edge column of the block
   * @param {bool} movingRight - direction the block travels in
   * @returns {void}
   */
  void draw(Display& display, int8_t left, bool movingRight) const;
};
