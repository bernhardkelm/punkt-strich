#include "clockFaces/glyphFrame.h"

namespace {

constexpr int16_t kFrameRows = sizeof(Frame::rows) / sizeof(Frame::rows[0]);
constexpr int16_t kFrameCols = sizeof(Frame::rows[0]) * 8;

}

Frame glyphFrame(const uint8_t* rows, uint8_t rowCount, uint8_t width, int8_t x, int8_t y) {
  Frame frame;

  for (uint8_t row = 0; row < rowCount; row++) {
    const int16_t frameY = static_cast<int16_t>(y + row);

    if (frameY < 0 || frameY >= kFrameRows) {
      continue;
    }

    for (uint8_t col = 0; col < width; col++) {
      const int16_t frameX = static_cast<int16_t>(x + col);
      const bool lit = ((rows[row] >> (width - 1 - col)) & 1U) != 0;

      if (!lit || frameX < 0 || frameX >= kFrameCols) {
        continue;
      }

      frame.rows[frameY] = static_cast<uint16_t>(frame.rows[frameY] | (1U << frameX));
    }
  }

  return frame;
}
