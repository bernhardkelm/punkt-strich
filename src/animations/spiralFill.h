#pragma once

#include "Display.h"

#include <cstdint>

/**
 * Sets one canvas pixel of an inward spiral fill. Index 0 is the bottom left corner, the path runs
 * right along the bottom row, up the right column, left along the top row and down the left
 * column, then turns one rectangular ring inward and repeats until the last index lands in the
 * centre.
 * @param {Display&} display - matrix to draw on
 * @param {uint8_t} n - fill index, below kCanvasWidth * kHeight, out of range is ignored
 * @param {bool} on - pixel state
 * @returns {void}
 */
void setSpiralFillPixel(Display& display, uint8_t n, bool on);
