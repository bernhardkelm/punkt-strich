#pragma once

#include <cstdint>

/** Rows of a 3x3 digit glyph, as wide as the 3x4 font */
constexpr uint8_t kSmallDigitRows = 3;

/**
 * Returns one row of a 3x3 digit glyph.
 * @param {uint8_t} digit - 0 to 9, anything else returns an empty row
 * @param {uint8_t} row - 0 (top) to 2 (bottom), anything else returns an empty row
 * @returns {uint8_t} - the three low bits of the row, bit 2 is the left column
 */
uint8_t smallDigitRow(uint8_t digit, uint8_t row);
