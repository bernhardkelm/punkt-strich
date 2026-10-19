#include "clockFaces/canvasFrame.h"

void drawCanvasFrame(Display& display, const Frame& frame) {
  for (int8_t y = 0; y < Display::kHeight; y++) {
    for (int8_t x = 0; x < Display::kCanvasWidth; x++) {
      display.setCanvasPixel(x, y, ((frame.rows[y] >> x) & 1U) != 0);
    }
  }
}

void drawViewportFrame(Display& display, const Frame& frame) {
  for (int8_t y = 0; y < Display::kHeight; y++) {
    for (int8_t x = 0; x < Display::kWidth; x++) {
      display.setPixel(x, y, ((frame.rows[y] >> x) & 1U) != 0);
    }
  }
}
