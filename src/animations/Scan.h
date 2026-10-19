#pragma once

#include "Animation.h"
#include "animations/linePlan.h"

/**
 * A page of text read line by line, the mirror of the code animation: the page stands there
 * complete, a cursor inverts the pixel it sits on while it runs along every written row, and once
 * the last row is read the page scrolls up and out to make room for the next one.
 */
class Scan : public Animation {
public:
  const char* name() const override;
  void begin(Display& display) override;
  void tick(Display& display, uint32_t nowMs) override;

private:
  static constexpr uint32_t kCursorStepMs = 40;
  static constexpr uint32_t kLinePauseMs = 150;
  static constexpr uint32_t kPageHoldMs = 300;
  static constexpr uint32_t kScrollStepMs = 60;

  /** What the page is doing, only READING puts the cursor on the matrix. */
  enum class Phase : uint8_t { READING, PAUSE, HOLD, SCROLL };

  uint8_t rows_[Display::kHeight] = {};
  uint8_t row_ = 0;
  uint8_t column_ = 0;
  uint8_t endColumn_ = 0;
  uint32_t nextStepMs_ = 0;
  Phase phase_ = Phase::READING;

  /**
   * Returns the lit columns of one planned line.
   * @param {const LinePlan&} plan - line to lay out
   * @returns {uint8_t} - column bitmask, 0 on a blank line
   */
  static uint8_t rowBits(const LinePlan& plan);

  /**
   * Returns the leftmost lit column of a row.
   * @param {uint8_t} bits - column bitmask of the row
   * @returns {uint8_t} - column, 0 on an empty row
   */
  static uint8_t firstLitColumn(uint8_t bits);

  /**
   * Returns the rightmost lit column of a row.
   * @param {uint8_t} bits - column bitmask of the row
   * @returns {uint8_t} - column, 0 on an empty row
   */
  static uint8_t lastLitColumn(uint8_t bits);

  /**
   * Returns the next row carrying at least one lit pixel, so blank rows are skipped.
   * @param {uint8_t} from - first row to look at
   * @returns {uint8_t} - row, Display::kHeight when no row below is written
   */
  uint8_t nextLitRow(uint8_t from) const;

  /**
   * Returns whether the page has scrolled off the matrix.
   * @returns {bool} - true while no row carries a lit pixel
   */
  bool pageEmpty() const;

  /** Plans all eight rows at once, so the page is complete before it is read. */
  void planPage();

  /**
   * Plans a page and starts reading its first written row.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void startPage(uint32_t nowMs);

  /**
   * Puts the cursor on the first lit pixel of the current row, or holds the page when no written
   * row is left.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void startRow(uint32_t nowMs);

  /**
   * Moves the cursor one column right and ends the row after its last lit pixel.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void readStep(uint32_t nowMs);

  /**
   * Waits out the pause after a read row, or holds the page when it was the last written one.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void endRow(uint32_t nowMs);

  /**
   * Moves to the next written row once the pause is over.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void advanceRow(uint32_t nowMs);

  /**
   * Scrolls the page up by one row, or starts the next page once it has left the matrix.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void scrollStep(uint32_t nowMs);

  /**
   * Draws the page, with the pixel under the cursor inverted while a row is being read.
   * @param {Display&} display - matrix to draw on
   * @returns {void}
   */
  void draw(Display& display) const;
};
