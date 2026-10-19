#include "clockFaces/minuteSquares.h"

namespace {

constexpr uint8_t kSide = 3;
constexpr uint8_t kCells = kSide * kSide;
constexpr uint8_t kBottomRow = 7;
constexpr uint8_t kTensX = 0;
constexpr uint8_t kOnesX = 4;

/**
 * Lights the first pixels of one square in fill order, bottom row first.
 * @param {Frame&} frame - frame to light the pixels in
 * @param {uint8_t} left - canvas column of the square's left edge
 * @param {uint8_t} pixels - lit pixels, more than kCells counts as kCells
 * @returns {void}
 */
void fillSquare(Frame& frame, uint8_t left, uint8_t pixels) {
  const uint8_t count = pixels > kCells ? kCells : pixels;

  for (uint8_t i = 0; i < count; i++) {
    const uint8_t y = static_cast<uint8_t>(kBottomRow - i / kSide);
    const uint8_t x = static_cast<uint8_t>(left + i % kSide);
    frame.rows[y] = static_cast<uint16_t>(frame.rows[y] | (1U << x));
  }
}

}

Frame minuteSquaresFrame(uint8_t tens, uint8_t ones) {
  Frame frame;
  fillSquare(frame, kTensX, tens);
  fillSquare(frame, kOnesX, ones);
  return frame;
}
