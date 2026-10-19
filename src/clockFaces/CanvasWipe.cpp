#include "clockFaces/CanvasWipe.h"

void CanvasWipe::start(Display& display, bool fromLeft, uint32_t nowMs) {
  display.freeze();
  freezeId_ = display.freezeCount();
  fromLeft_ = fromLeft;
  columns_ = 0;
  running_ = true;
  dueMs_ = nowMs + kColumnMs;
}

void CanvasWipe::tick(Display& display, uint32_t nowMs) {
  if (!running_ || static_cast<int32_t>(nowMs - dueMs_) < 0) {
    return;
  }

  if (!ownsReveal(display)) {
    running_ = false;
    return;
  }

  columns_++;

  if (columns_ >= Display::kCanvasWidth) {
    stop(display);
    return;
  }

  display.setReveal(mask(columns_, fromLeft_));
  dueMs_ = nowMs + kColumnMs;
}

void CanvasWipe::stop(Display& display) {
  if (ownsReveal(display)) {
    display.endReveal();
  }

  running_ = false;
}

Frame CanvasWipe::mask(uint8_t columns, bool fromLeft) {
  const uint32_t canvasBits = (1U << Display::kCanvasWidth) - 1U;
  const uint32_t leftBits = (1U << columns) - 1U;
  const uint32_t rightBits = canvasBits & ~((1U << (Display::kCanvasWidth - columns)) - 1U);
  const uint16_t bits = static_cast<uint16_t>(fromLeft ? leftBits : rightBits);
  Frame frame;

  for (uint16_t& row : frame.rows) {
    row = bits;
  }

  return frame;
}

bool CanvasWipe::ownsReveal(const Display& display) const {
  return running_ && display.revealing() && display.freezeCount() == freezeId_;
}
