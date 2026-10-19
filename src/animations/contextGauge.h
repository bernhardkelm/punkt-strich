#pragma once

#include "Display.h"

#include <cstdint>

/**
 * Draws the bottom-row context window gauge, call before drawing the rest of the frame.
 * @param {Display&} display - matrix to draw on
 * @param {uint8_t} percent - 0 to 100 share of the context window filled
 * @returns {void}
 */
void drawContextGauge(Display& display, uint8_t percent);

/**
 * Returns how many rows are available for the animation itself.
 * @param {uint8_t} percent - context window fill, 0 hides the gauge
 * @returns {uint8_t} - 8 rows normally, 7 when the gauge reserves row 7
 */
uint8_t usableRows(uint8_t percent);
