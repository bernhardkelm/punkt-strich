#pragma once

#include "Display.h"

#include <cstdint>

/**
 * Returns whether the soft blinking marker is lit, 3.5 s on and 0.5 s off.
 * @param {uint32_t} elapsed - milliseconds since the marker appeared
 * @returns {bool} - true while the marker pixel is on
 */
bool markerLit(uint32_t elapsed);

/**
 * Draws the marker pixel in the top right corner over a dark matrix with the context gauge.
 * @param {Display&} display - matrix to draw on
 * @param {bool} lit - marker pixel state
 * @param {uint8_t} maxBrightness - brightness ceiling, the marker uses half of it
 * @param {uint8_t} contextPercent - context gauge value, 0 hides the gauge
 * @returns {void}
 */
void drawMarker(Display& display, bool lit, uint8_t maxBrightness, uint8_t contextPercent);
