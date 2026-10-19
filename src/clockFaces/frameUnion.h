#pragma once

#include "Frame.h"

#include <cstddef>
#include <cstdint>

/**
 * Returns the pixels lit in either of two frames.
 * @param {const Frame&} a - first frame
 * @param {const Frame&} b - second frame
 * @returns {Frame} - frame with every pixel of both set
 */
inline Frame frameUnion(const Frame& a, const Frame& b) {
  Frame frame;

  for (size_t y = 0; y < sizeof(frame.rows) / sizeof(frame.rows[0]); y++) {
    frame.rows[y] = static_cast<uint16_t>(a.rows[y] | b.rows[y]);
  }

  return frame;
}
