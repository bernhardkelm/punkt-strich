#pragma once

#include "Animation.h"

/** Usage view: the share of the 5 hour usage limit spent as a gauge filling the matrix bottom up. */
class UsageView : public Animation {
public:
  const char* name() const override;
  void begin(Display& display) override;
  void tick(Display& display, uint32_t nowMs) override;

  /**
   * Sets the usage the gauge shows, the next tick redraws when it changed.
   * @param {uint8_t} percent - 0 shows the placeholder pixel, 1 to 100 fill the matrix
   * @returns {void}
   */
  void setPercent(uint8_t percent);

private:
  static constexpr uint8_t kPixelCount = Display::kWidth * Display::kHeight;
  static constexpr uint8_t kBlinkPercent = 80;
  static constexpr uint8_t kPulsePercent = 95;
  static constexpr uint32_t kBlinkPeriodMs = 2000;
  static constexpr uint32_t kBlinkOnMs = 1500;
  static constexpr uint32_t kPulsePeriodMs = 1000;
  static constexpr uint32_t kPulseStepMs = 40;
  static constexpr uint8_t kDimDivisor = 4;

  uint32_t startMs_ = 0;
  uint8_t percent_ = 0;
  uint8_t pixels_ = 0;
  uint8_t brightness_ = 0;
  bool lit_ = true;

  /**
   * Returns how many pixels the fill covers.
   * @param {uint8_t} percent - 0 to 100 share of the limit spent
   * @returns {uint8_t} - 0 to 64, 0 when no value was reported
   */
  static uint8_t pixelsFor(uint8_t percent);

  /**
   * Returns whether the topmost filled row is lit, it blinks above kBlinkPercent.
   * @param {uint32_t} phase - milliseconds since begin()
   * @param {uint8_t} percent - 0 to 100 share of the limit spent
   * @returns {bool} - true while the row shows
   */
  static bool topRowLit(uint32_t phase, uint8_t percent);

  /**
   * Returns the dim level the placeholder pixel and the pulse floor use.
   * @returns {uint8_t} - 1 to 15
   */
  uint8_t dim() const;

  /**
   * Returns the brightness for a point in the period, a triangle wave above kPulsePercent.
   * @param {uint32_t} phase - milliseconds since begin()
   * @param {uint8_t} percent - 0 to 100 share of the limit spent
   * @returns {uint8_t} - 0 to 15
   */
  uint8_t brightnessAt(uint32_t phase, uint8_t percent) const;

  /**
   * Draws the fill, bottom row first and left to right within a row.
   * @param {Display&} display - matrix to draw on
   * @param {uint8_t} pixels - pixels to light, 0 draws the placeholder pixel
   * @param {bool} lit - false blanks the topmost filled row
   * @returns {void}
   */
  static void draw(Display& display, uint8_t pixels, bool lit);
};
