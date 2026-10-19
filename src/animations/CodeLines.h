#pragma once

#include "Animation.h"
#include "animations/linePlan.h"

/**
 * Lines of code typed from the top row down, lines open and close indented blocks, the page
 * clears once the last row is written.
 */
class CodeLines : public Animation {
public:
  const char* name() const override;
  void begin(Display& display) override;
  void tick(Display& display, uint32_t nowMs) override;

private:
  static constexpr uint32_t kTypeStepMs = 70;
  static constexpr uint32_t kLinePauseMs = 250;
  static constexpr uint32_t kRestartHoldMs = 500;

  /** What the current row is doing, PAUSE and HOLD leave the picture untouched. */
  enum class Phase : uint8_t { TYPING, PAUSE, HOLD };

  uint8_t rows_[Display::kHeight] = {};
  LinePlan plan_ = {};
  uint8_t row_ = 0;
  uint8_t column_ = 0;
  uint8_t endColumn_ = 0;
  uint8_t indent_ = 0;
  uint32_t nextStepMs_ = 0;
  Phase phase_ = Phase::TYPING;

  /**
   * Plans the current row and starts typing it, or leaves the row blank.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void startRow(uint32_t nowMs);

  /**
   * Lights the next pixel of the current line and ends the row on its last column.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void typeStep(uint32_t nowMs);

  /**
   * Waits out the gap after a finished row, or holds the full page when it was the last one.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void endRow(uint32_t nowMs);

  /**
   * Moves to the next row once the gap is over.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void advanceRow(uint32_t nowMs);

  /**
   * Clears the page and starts over at the top row with fresh lines and no indent.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void restart(uint32_t nowMs);

  /**
   * Draws every row written so far.
   * @param {Display&} display - matrix to draw on
   * @returns {void}
   */
  void draw(Display& display) const;
};
