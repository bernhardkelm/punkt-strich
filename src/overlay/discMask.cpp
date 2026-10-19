#include "overlay/discMask.h"

#include "board/boardConfig.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace {

constexpr uint8_t kRows = 8;
constexpr uint16_t kCanvasMask = static_cast<uint16_t>((1u << Board::kCanvasWidth) - 1u);

/**
 * Returns the squared distance of a pixel from the overlay origin in doubled units.
 * @param {uint8_t} x - column, 0 is left
 * @param {uint8_t} y - row, 0 is top
 * @param {uint8_t} originX - columns the origin is shifted right
 * @returns {uint16_t} - dx * dx + dy * dy with dx = 2(x - originX) - 7 and dy = 2y + 1
 */
uint16_t squaredDistance(uint8_t x, uint8_t y, uint8_t originX) {
  const int16_t dx = static_cast<int16_t>(2 * (x - originX) - 7);
  const int16_t dy = static_cast<int16_t>(2 * y + 1);
  return static_cast<uint16_t>(dx * dx + dy * dy);
}

/**
 * Returns the bit mask of one column.
 * @param {uint8_t} x - column, 0 is left
 * @returns {uint16_t} - single bit at position x
 */
uint16_t columnBit(uint8_t x) {
  return static_cast<uint16_t>(1u << x);
}

/**
 * Returns the bit mask of the columns firstCol to lastCol, limited to the canvas.
 * @param {uint8_t} firstCol - first column kept
 * @param {uint8_t} lastCol - last column kept, inclusive
 * @returns {uint16_t} - mask with one bit per kept column
 */
uint16_t columnRange(uint8_t firstCol, uint8_t lastCol) {
  uint16_t range = 0;

  for (uint8_t x = firstCol; x <= lastCol && x < Board::kCanvasWidth; ++x) {
    range |= columnBit(x);
  }

  return range;
}

}

Frame discMask(uint16_t r2, uint8_t originX, uint8_t firstCol, uint8_t lastCol) {
  // the closest pixel sits at d2 = 2, so the exclusive inner radius of 0 drops nothing
  return ringMask(r2, 0, originX, firstCol, lastCol);
}

Frame ringMask(uint16_t r2Outer, uint16_t r2Inner, uint8_t originX, uint8_t firstCol,
               uint8_t lastCol) {
  Frame frame;
  const uint16_t kept = columnRange(firstCol, lastCol);

  for (uint8_t y = 0; y < kRows; ++y) {
    for (uint8_t x = 0; x < Board::kCanvasWidth; ++x) {
      const uint16_t d2 = squaredDistance(x, y, originX);

      if ((kept & columnBit(x)) != 0 && d2 > r2Inner && d2 <= r2Outer) {
        frame.rows[y] |= columnBit(x);
      }
    }
  }

  return frame;
}

Frame keyline(const Frame& frame, uint8_t firstCol, uint8_t lastCol) {
  Frame outline;
  const uint16_t kept = columnRange(firstCol, lastCol);

  for (uint8_t y = 0; y < kRows; ++y) {
    const uint16_t row = frame.rows[y];
    // the mask drops the bit shifted past the last canvas column
    uint16_t dilated = static_cast<uint16_t>((row | (row << 1) | (row >> 1)) & kCanvasMask);

    if (y > 0) {
      dilated |= frame.rows[y - 1];
    }

    if (y + 1 < kRows) {
      dilated |= frame.rows[y + 1];
    }

    outline.rows[y] = static_cast<uint16_t>(dilated & ~row & kept);
  }

  return outline;
}

float cornerRadius(uint8_t originX, uint8_t firstCol, uint8_t lastCol) {
  const int16_t toFirst = 2 * (firstCol - originX) - 7;
  const int16_t toLast = 2 * (lastCol - originX) - 7;
  const float dx = static_cast<float>(std::max(std::abs(toFirst), std::abs(toLast)));
  const float dy = 15.0f;
  return std::sqrt(dx * dx + dy * dy);
}

uint16_t squaredRadius(float radius) {
  return static_cast<uint16_t>(radius * radius + 0.5f);
}

Frame merged(const Frame& a, const Frame& b) {
  Frame out;

  for (uint8_t y = 0; y < kRows; ++y) {
    out.rows[y] = static_cast<uint16_t>(a.rows[y] | b.rows[y]);
  }

  return out;
}
