#pragma once

#include "Display.h"

#include <cstdint>

/**
 * Draws the border of the centred square for one ring radius.
 * @param {Display&} display - matrix to draw on
 * @param {uint8_t} radius - 0 is the 2x2 centre, 3 the outer frame
 * @returns {void}
 */
void drawRing(Display& display, uint8_t radius);
