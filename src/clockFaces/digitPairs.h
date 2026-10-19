#pragma once

#include "Frame.h"

#include <cstdint>

/** Columns from the left edge of the tens digit to the left edge of the ones digit */
constexpr uint8_t kDigitPairStep = 4;

/**
 * Returns a frame holding a two-digit value in the 3x4 font with a leading zero, pixels that fall
 * outside the frame are dropped.
 * @param {uint8_t} value - 0 to 99
 * @param {int8_t} x - canvas column of the tens digit's left edge
 * @param {int8_t} y - canvas row of the top glyph edge
 * @returns {Frame} - frame with the digit pixels set
 */
Frame digitPairFrame(uint8_t value, int8_t x, int8_t y);

/**
 * Returns a frame holding a two-digit value in the 3x3 font with a leading zero, pixels that fall
 * outside the frame are dropped.
 * @param {uint8_t} value - 0 to 99
 * @param {int8_t} x - canvas column of the tens digit's left edge
 * @param {int8_t} y - canvas row of the top glyph edge
 * @returns {Frame} - frame with the digit pixels set
 */
Frame smallPairFrame(uint8_t value, int8_t x, int8_t y);
