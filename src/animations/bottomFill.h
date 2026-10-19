#pragma once

#include "Display.h"

#include <cstdint>

/**
 * Sets one viewport pixel of a bottom-up, left-to-right fill, row 0 of the fill is the viewport
 * bottom row.
 * @param {Display&} display - matrix to draw on
 * @param {uint8_t} n - fill index, 0 is the bottom left viewport pixel
 * @param {bool} on - pixel state
 * @returns {void}
 */
void setBottomFillPixel(Display& display, uint8_t n, bool on);

/**
 * Sets one canvas pixel of a bottom-up, left-to-right fill, row 0 of the fill is the canvas
 * bottom row.
 * @param {Display&} display - matrix to draw on
 * @param {uint8_t} n - fill index, 0 is the bottom left canvas pixel
 * @param {bool} on - pixel state
 * @returns {void}
 */
void setCanvasBottomFillPixel(Display& display, uint8_t n, bool on);
