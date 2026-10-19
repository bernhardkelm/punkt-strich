#pragma once

#include "Animation.h"
#include "animations/wifiSymbol.h"

/** Wi-Fi symbol whose arcs light up one after another, then a dark pause, looping. */
class WifiSearch : public Animation {
public:
  const char* name() const override;
  void begin(Display& display) override;
  void tick(Display& display, uint32_t nowMs) override;

private:
  static constexpr uint32_t kArcStepMs = 350;
  static constexpr uint32_t kPauseMs = 500;
  /** the dot alone plus one step per arc */
  static constexpr uint8_t kStepCount = kWifiArcCount + 1;
  static constexpr uint32_t kPeriodMs = kStepCount * kArcStepMs + kPauseMs;

  uint32_t startMs_ = 0;
  uint8_t frame_ = 0;

  /**
   * Maps a point in the loop to a frame index.
   * @param {uint32_t} phase - milliseconds into the period
   * @returns {uint8_t} - arcs to light, kStepCount for the dark pause
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
