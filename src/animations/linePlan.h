#pragma once

#include "Display.h"

#include <cstdint>

/** One planned row of code, everything drawing and the next plan need. */
struct LinePlan {
  /** Dark columns before the first pixel, one per block level */
  uint8_t indent = 0;
  /** Lit columns including the gap column, 0 on a blank row */
  uint8_t length = 0;
  /** Column left dark inside the line, -1 when the line has no gap */
  int8_t gapColumn = -1;
  /** Line opens a block, the next line sits one level deeper */
  bool opener = false;
  /** One pixel line at the outer indent, closes the current block */
  bool closer = false;
  bool blank = false;
};

/**
 * Picks the shape of the next row from the current indent level and the flags of the row before
 * it.
 * @param {const LinePlan&} previous - plan of the row above, a default plan above the top row
 * @param {uint8_t} indent - block level the row starts at
 * @param {bool} firstRow - true on the top row, which is never left blank
 * @returns {LinePlan} - the row to draw, or a blank row
 */
LinePlan planLine(const LinePlan& previous, uint8_t indent, bool firstRow);

/**
 * Returns the block level the row below a planned one starts at.
 * @param {const LinePlan&} plan - plan of the row above
 * @returns {uint8_t} - one level deeper after an opener, the plan's own indent otherwise
 */
uint8_t nextIndent(const LinePlan& plan);
