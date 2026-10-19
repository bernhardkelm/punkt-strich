#include "SplitPicture.h"

#include <utility>

SplitPicture::SplitPicture(const Settings& settings, std::unique_ptr<Animation> picture)
    : sideClock_(settings), picture_(std::move(picture)) {
}

const char* SplitPicture::name() const {
  return "split";
}

void SplitPicture::begin(Display& display) {
  display.clearCanvas();
  display.setViewportX(0);
  sideClock_.setMaxBrightness(maxBrightness());
  sideClock_.begin(display);
  display.setViewportX(Display::kViewportX);
  display.show();
  picture_->setMaxBrightness(maxBrightness());
  picture_->begin(display);
}

void SplitPicture::tick(Display& display, uint32_t nowMs) {
  display.setViewportX(0);
  sideClock_.tick(display, nowMs);
  display.setViewportX(Display::kViewportX);
  picture_->tick(display, nowMs);
}
