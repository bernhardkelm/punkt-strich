#include "animations/ResetFill.h"

#include "animations/bottomFill.h"

const char* ResetFill::name() const {
  return "reset";
}

void ResetFill::begin(Display& display) {
  draw(display);
}

void ResetFill::tick(Display& display, uint32_t) {
  if (pixels_ == drawn_) {
    return;
  }

  draw(display);
}

void ResetFill::setProgress(uint8_t pixels) {
  const uint8_t total = Display::kCanvasWidth * Display::kHeight;
  pixels_ = pixels > total ? total : pixels;
}

void ResetFill::draw(Display& display) {
  display.clearCanvas();

  for (uint8_t n = 0; n < pixels_; n++) {
    setCanvasBottomFillPixel(display, n, true);
  }

  display.show();
  drawn_ = pixels_;
}
