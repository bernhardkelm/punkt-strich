#pragma once

#include "Display.h"

#include <cstdint>

/**
 * Draws the lit pixels of a 3-column digit stretched to a height on canvas coordinates.
 * Display row r samples glyph row floor(r * 4 / rows), unlit pixels stay untouched.
 * @param {Display&} display - canvas to draw on, cleared by the caller
 * @param {uint8_t} digit - 0 to 9, anything else draws nothing
 * @param {int8_t} x - canvas column of the left glyph edge
 * @param {int8_t} y - canvas row of the top glyph edge
 * @param {uint8_t} rows - height in canvas rows, 4 is the plain glyph
 * @returns {void}
 */
void drawScaledDigit(Display& display, uint8_t digit, int8_t x, int8_t y, uint8_t rows);
