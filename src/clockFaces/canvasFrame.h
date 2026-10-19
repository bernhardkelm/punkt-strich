#pragma once

#include "Display.h"
#include "Frame.h"

/**
 * Sets every canvas pixel of the animation frame to the matching bit of a frame, so it replaces
 * the whole canvas, call show() to apply.
 * @param {Display&} display - canvas to draw on
 * @param {const Frame&} frame - picture to copy, bit x of row y is canvas column x
 * @returns {void}
 */
void drawCanvasFrame(Display& display, const Frame& frame);

/**
 * Sets every viewport pixel of the animation frame to the matching bit of a frame, so it replaces
 * the whole viewport, call show() to apply.
 * @param {Display&} display - viewport to draw on
 * @param {const Frame&} frame - picture to copy, bit x of row y is viewport column x
 * @returns {void}
 */
void drawViewportFrame(Display& display, const Frame& frame);
