#pragma once

#include "Frame.h"

#include <cstdint>

/**
 * Returns the two 3x3 squares of the SQUARES split clock on rows 5 to 7, the minute tens at
 * column 0 and the minute ones at column 4, each lit with as many pixels as it counts. A square
 * fills like a glass, the bottom row from left to right first, then the rows above.
 * @param {uint8_t} tens - lit pixels of the left square, more than 9 counts as 9
 * @param {uint8_t} ones - lit pixels of the right square, more than 9 counts as 9
 * @returns {Frame} - frame with the square pixels set
 */
Frame minuteSquaresFrame(uint8_t tens, uint8_t ones);
