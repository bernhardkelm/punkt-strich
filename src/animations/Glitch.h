#pragma once

#include "Animation.h"

/** A thick X with three short glitch bursts that settles into a dim X, held while it is shown. */
class Glitch : public Animation {
public:
  const char* name() const override;
  void begin(Display& display) override;
  void tick(Display& display, uint32_t nowMs) override;
  void applyBrightness(Display& display, uint32_t fadeMs) override;
  bool finished() const override;

private:
  static constexpr uint32_t kDurationMs = 600;
  static constexpr uint8_t kBurstCount = 3;
  static constexpr uint32_t kBurstStartMs[kBurstCount] = {0, 200, 400};
  static constexpr uint32_t kBurstEndMs[kBurstCount] = {120, 320, 520};
  static constexpr uint8_t kNoBurstYet = 0xFF;

  uint32_t startMs_ = 0;
  uint8_t burst_ = kNoBurstYet;
  bool finished_ = false;
  bool held_ = false;
  uint8_t glitchRows_[Display::kHeight] = {};

  /**
   * Maps elapsed time to a burst index.
   * @param {uint32_t} elapsed - milliseconds since begin()
   * @returns {uint8_t} - 0 outside a burst, 1 to kBurstCount inside one
   */
  static uint8_t burstAt(uint32_t elapsed);

  /**
   * Advances to the frame for the given elapsed time, redrawing only on change.
   * @param {Display&} display - matrix to draw on
   * @param {uint32_t} elapsed - milliseconds since begin()
   * @returns {void}
   */
  void step(Display& display, uint32_t elapsed);

  /**
   * Draws the held X at half brightness once the bursts are over, only on the first call.
   * @param {Display&} display - matrix to draw on
   * @returns {void}
   */
  void hold(Display& display);

  /**
   * Draws the plain X or the current glitch frame.
   * @param {Display&} display - matrix to draw on
   * @param {bool} glitched - true to draw the stored glitch frame
   * @returns {void}
   */
  void draw(Display& display, bool glitched) const;

  /**
   * Draws the plain X.
   * @param {Display&} display - matrix to draw on
   * @param {uint8_t} brightness - level to show it at
   * @returns {void}
   */
  void drawPlain(Display& display, uint8_t brightness) const;
};
