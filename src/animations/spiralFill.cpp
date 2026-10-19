#include "animations/spiralFill.h"

namespace {

constexpr uint8_t kSpiralLength = Display::kCanvasWidth * Display::kHeight;
constexpr int8_t kRingCount = static_cast<int8_t>(
  (Display::kCanvasWidth < Display::kHeight ? Display::kCanvasWidth : Display::kHeight) / 2);

static_assert(Display::kHeight >= 2, "the innermost ring needs two rows");

struct SpiralPath {
  int8_t x[kSpiralLength];
  int8_t y[kSpiralLength];
};

/**
 * Builds the spiral path over the canvas, one rectangular ring per lap from the border
 * inward, each lap running right along its bottom row, up its right column, left along its top
 * row and down its left column, stopping one pixel short of where the lap started so the next lap
 * begins one ring in.
 * @returns {SpiralPath} - every canvas position in fill order
 */
constexpr SpiralPath buildSpiralPath() {
  SpiralPath path = {};
  uint8_t n = 0;

  for (int8_t ring = 0; ring < kRingCount; ring++) {
    const int8_t left = ring;
    const int8_t right = static_cast<int8_t>(Display::kCanvasWidth - 1 - ring);
    const int8_t top = ring;
    const int8_t bottom = static_cast<int8_t>(Display::kHeight - 1 - ring);

    for (int8_t x = left; x <= right; x++) {
      path.x[n] = x;
      path.y[n] = bottom;
      n++;
    }

    for (int8_t y = static_cast<int8_t>(bottom - 1); y >= top; y--) {
      path.x[n] = right;
      path.y[n] = y;
      n++;
    }

    for (int8_t x = static_cast<int8_t>(right - 1); x >= left; x--) {
      path.x[n] = x;
      path.y[n] = top;
      n++;
    }

    for (int8_t y = static_cast<int8_t>(top + 1); y < bottom; y++) {
      path.x[n] = left;
      path.y[n] = y;
      n++;
    }
  }

  return path;
}

constexpr SpiralPath kSpiralPath = buildSpiralPath();

}

void setSpiralFillPixel(Display& display, uint8_t n, bool on) {
  if (n >= kSpiralLength) {
    return;
  }

  display.setCanvasPixel(kSpiralPath.x[n], kSpiralPath.y[n], on);
}
