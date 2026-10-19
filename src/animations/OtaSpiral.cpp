#include "animations/OtaSpiral.h"

#include "animations/spiralFill.h"

const char* OtaSpiral::name() const {
  return "ota";
}

void OtaSpiral::begin(Display& display) {
  draw(display);
}

void OtaSpiral::tick(Display& display, uint32_t) {
  if (pixels_ == drawn_) {
    return;
  }

  draw(display);
}

void OtaSpiral::setProgress(uint8_t pixels) {
  const uint8_t total = Display::kCanvasWidth * Display::kHeight;
  pixels_ = pixels > total ? total : pixels;
}

void OtaSpiral::draw(Display& display) {
  display.setBrightness(maxBrightness());
  display.clearCanvas();

  for (uint8_t n = 0; n < pixels_; n++) {
    setSpiralFillPixel(display, n, true);
  }

  display.show();
  drawn_ = pixels_;
}
