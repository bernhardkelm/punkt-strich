#pragma once

#include "Frame.h"

#include <cstdint>

/**
 * Returns a frame holding one glyph, pixels that fall outside the frame are dropped.
 * @param {const uint8_t*} rows - glyph rows from the top, the highest width bit is the left column
 * @param {uint8_t} rowCount - rows in the glyph
 * @param {uint8_t} width - columns in the glyph, at most 8
 * @param {int8_t} x - canvas column of the left glyph edge
 * @param {int8_t} y - canvas row of the top glyph edge
 * @returns {Frame} - frame with the glyph pixels set
 */
Frame glyphFrame(const uint8_t* rows, uint8_t rowCount, uint8_t width, int8_t x, int8_t y);
