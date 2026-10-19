#pragma once

#include "Display.h"
#include "clockFaces/clockFrame.h"
#include "settings/Settings.h"

#include <cstdint>

/** Path position of the full Staircase face, where the squash from the Tall face ends */
constexpr uint8_t kSquashSteps = kTallRows - kStaircaseRows;

/**
 * Returns the path position of the split clock's picture, the last clock frame before the split
 * picture wipes in. Position 0 is the full Tall face and kSquashSteps the full Staircase face,
 * every position after it belongs to the style: STAGGER slides the minute pair 8 columns left,
 * DIGITS switches the minutes to the 3x3 font and slides them 9 columns left, SQUARES and BAR
 * drop the minute pair off the bottom edge and then fill the squares or the bar pixel by pixel.
 * @param {SplitClock} style - split clock style the path leads to
 * @param {uint8_t} minute - 0 to 59, SQUARES and BAR take one step per lit minute pixel
 * @returns {uint8_t} - last clock position of the path
 */
uint8_t splitPathEnd(SplitClock style, uint8_t minute);

/**
 * Returns the wait before the step into a path position, the step back out of it waits as long.
 * @param {SplitClock} style - split clock style the path leads to
 * @param {uint8_t} minute - 0 to 59, the minute the path is drawn with
 * @param {uint8_t} position - 1 to splitPathEnd()
 * @returns {uint32_t} - milliseconds
 */
uint32_t splitPathStepMs(SplitClock style, uint8_t minute, uint8_t position);

/**
 * Replaces the canvas with the clock frame of a path position, the last one being exactly
 * splitClockFrame() of the style. Call show() to apply.
 * @param {Display&} display - canvas to draw on
 * @param {SplitClock} style - split clock style the path leads to
 * @param {uint8_t} hour - 0 to 23
 * @param {uint8_t} minute - 0 to 59
 * @param {uint8_t} position - 0 (full Tall face) to splitPathEnd()
 * @returns {void}
 */
void drawSplitPath(Display& display, SplitClock style, uint8_t hour, uint8_t minute,
                   uint8_t position);
