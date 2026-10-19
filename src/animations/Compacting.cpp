#include "animations/Compacting.h"

#include "animations/rings.h"

#include <Arduino.h>

const char* Compacting::name() const {
  return "compacting";
}

void Compacting::begin(Display& display) {
  startMs_ = millis();
  frame_ = 0;
  draw(display, frame_);
}

void Compacting::tick(Display& display, uint32_t nowMs) {
  const uint32_t elapsed = (nowMs - startMs_) % (kFrameCount * kFrameMs);
  const uint8_t frame = static_cast<uint8_t>(elapsed / kFrameMs);

  if (frame == frame_) {
    return;
  }

  frame_ = frame;
  draw(display, frame);
}

void Compacting::draw(Display& display, uint8_t frame) const {
  display.setBrightness(maxBrightness());
  display.clear();

  switch (frame) {
    case 0:
      drawRing(display, 3);
      break;
    case 1:
      drawRing(display, 3);
      drawRing(display, 2);
      break;
    case 2:
      drawRing(display, 2);
      break;
    case 3:
      drawRing(display, 2);
      drawRing(display, 1);
      break;
    case 4:
      drawRing(display, 1);
      break;
    default:
      drawRing(display, 0);
      break;
  }

  display.show();
}
