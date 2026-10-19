#pragma once

#include "Display.h"

#include <cstdint>

/**
 * Draws the circular ring of one radius, every pixel whose euclidean distance from the centre is
 * within 0.5 of it.
 * @param {Display&} display - matrix to draw on
 * @param {int8_t} centerX - ring centre column
 * @param {int8_t} centerY - ring centre row
 * @param {uint8_t} radius - ring radius in pixels
 * @returns {void}
 */
void drawRipple(Display& display, int8_t centerX, int8_t centerY, uint8_t radius);
