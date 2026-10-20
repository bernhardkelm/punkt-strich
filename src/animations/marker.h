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
 * Returns the level the marker picture runs at, half the ceiling and at least 1.
 * @param {uint8_t} maxBrightness - brightness ceiling, 0 to 15
 * @returns {uint8_t} - marker brightness, 1 to 7
 */
uint8_t markerBrightness(uint8_t maxBrightness);

/**
 * Draws the marker pixel in the top right corner over a dark matrix with the context gauge.
 * @param {Display&} display - matrix to draw on
 * @param {bool} lit - marker pixel state
 * @param {uint8_t} maxBrightness - brightness ceiling, the marker uses half of it
 * @param {uint8_t} contextPercent - context gauge value, 0 hides the gauge
 * @returns {void}
 */
void drawMarker(Display& display, bool lit, uint8_t maxBrightness, uint8_t contextPercent);
