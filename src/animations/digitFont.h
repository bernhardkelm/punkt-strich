#pragma once

#include "Display.h"

#include <cstdint>

constexpr uint8_t kDigitRows = 4;
constexpr uint8_t kDigitCols = 3;

/**
 * Returns one row of a 3x4 digit glyph.
 * @param {uint8_t} digit - 0 to 9, anything else returns an empty row
 * @param {uint8_t} row - 0 (top) to 3 (bottom), anything else returns an empty row
 * @returns {uint8_t} - the three low bits of the row, bit 2 is the left column
 */
uint8_t digitRow(uint8_t digit, uint8_t row);

/**
 * Draws the lit pixels of a 3x4 digit, unlit pixels of the glyph box stay untouched.
 * @param {Display&} display - matrix to draw on, cleared by the caller
 * @param {uint8_t} digit - 0 to 9, anything else draws nothing
 * @param {int8_t} x - column of the left glyph edge
 * @param {int8_t} y - row of the top glyph edge
 * @returns {void}
 */
void drawDigit(Display& display, uint8_t digit, int8_t x, int8_t y);
