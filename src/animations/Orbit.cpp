#include "animations/Orbit.h"

#include <Arduino.h>

namespace {

constexpr int8_t kLastRow = Display::kHeight - 1;

}

Orbit::Orbit(bool fullCanvas)
    : fullCanvas_(fullCanvas),
      width_(fullCanvas ? Display::kCanvasWidth : Display::kWidth),
      pathLength_(static_cast<uint8_t>(2 * (width_ + Display::kHeight) - 4)) {
}

const char* Orbit::name() const {
  return "orbit";
}

void Orbit::begin(Display& display) {
  startMs_ = millis();
  head_ = 0;
  draw(display, head_);
}

void Orbit::tick(Display& display, uint32_t nowMs) {
  const uint8_t head = static_cast<uint8_t>(((nowMs - startMs_) / kStepMs) % pathLength_);

  if (head == head_) {
    return;
  }

  head_ = head;
  draw(display, head);
}

void Orbit::positionAt(uint8_t step, int8_t& x, int8_t& y) const {
  const int8_t lastColumn = static_cast<int8_t>(width_ - 1);
  const uint8_t topSteps = width_;
  const uint8_t rightSteps = static_cast<uint8_t>(topSteps + kLastRow);
  const uint8_t bottomSteps = static_cast<uint8_t>(rightSteps + lastColumn);

  if (step < topSteps) {
    x = static_cast<int8_t>(step);
    y = 0;
    return;
  }

  if (step < rightSteps) {
    x = lastColumn;
    y = static_cast<int8_t>(step - lastColumn);
    return;
  }

  if (step < bottomSteps) {
    x = static_cast<int8_t>(bottomSteps - 1 - step);
    y = kLastRow;
    return;
  }

  x = 0;
  y = static_cast<int8_t>(pathLength_ - step);
}

void Orbit::draw(Display& display, uint8_t head) const {
  display.setBrightness(maxBrightness());

  if (fullCanvas_) {
    display.clearCanvas();
  } else {
    display.clear();
  }

  for (uint8_t i = 0; i < kCometLength; i++) {
    const uint8_t step = static_cast<uint8_t>((head + pathLength_ - i) % pathLength_);
    int8_t x = 0;
    int8_t y = 0;
    positionAt(step, x, y);

    if (fullCanvas_) {
      display.setCanvasPixel(x, y, true);
    } else {
      display.setPixel(x, y, true);
    }
  }

  display.show();
}
