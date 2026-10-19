#pragma once

#include "Animation.h"

/** Confirmation: rings bloom, the matrix pulses and drains row by row until it is dark. */
class Done : public Animation {
public:
  const char* name() const override;
  void begin(Display& display) override;
  void tick(Display& display, uint32_t nowMs) override;
  bool finished() const override;

private:
  static constexpr uint32_t kRingMs = 160;
  static constexpr uint32_t kDimMs = 640;
  static constexpr uint32_t kPeakMs = 800;
  static constexpr uint32_t kHoldMs = 960;
  static constexpr uint32_t kDrainStartMs = 2400;
  static constexpr uint32_t kDrainRowMs = 125;
  static constexpr uint32_t kDurationMs = 3400;
  static constexpr uint8_t kRingCount = 4;
  static constexpr uint8_t kDimPercent = 50;
  static constexpr uint8_t kPeakPercent = 100;
  static constexpr uint8_t kHoldPercent = 60;
  static constexpr uint8_t kDrainFrame = 7;
  static constexpr uint8_t kEndFrame = kDrainFrame + Display::kHeight;

  uint32_t startMs_ = 0;
  uint8_t frame_ = 0;
  bool finished_ = false;

  /**
   * Maps the elapsed time to a frame index.
   * @param {uint32_t} elapsed - milliseconds since begin()
   * @returns {uint8_t} - 0 to kEndFrame
   */
  static uint8_t frameAt(uint32_t elapsed);

  /**
   * Scales the brightness ceiling.
   * @param {uint8_t} percent - share of the ceiling to use
   * @returns {uint8_t} - rounded brightness level
   */
  uint8_t scaled(uint8_t percent) const;

  /**
   * Draws one frame.
   * @param {Display&} display - matrix to draw on
   * @param {uint8_t} frame - frame index from frameAt()
   * @returns {void}
   */
  void draw(Display& display, uint8_t frame) const;
};
