#pragma once

#include <cstdint>

/** One bit per pixel of the canvas, rows[y] bit x is column x, row 0 is the top. */
struct Frame {
  uint16_t rows[8] = {};
};
