#pragma once

#include "Display.h"

#include <cstdint>

/**
 * Draws the border of the rectangle spanned by two opposite corners, in any corner order.
 * @param {Display&} display - matrix to draw on
 * @param {int8_t} x0 - first corner column
 * @param {int8_t} y0 - first corner row
 * @param {int8_t} x1 - opposite corner column
 * @param {int8_t} y1 - opposite corner row
 * @returns {void}
 */
void drawRectOutline(Display& display, int8_t x0, int8_t y0, int8_t x1, int8_t y1);
