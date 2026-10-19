#include "animations/ripple.h"

void drawRipple(Display& display, int8_t centerX, int8_t centerY, uint8_t radius) {
  const int16_t span = static_cast<int16_t>(radius);
  // squared distances keep the ring off the float unit, |d - r| <= 0.5 is r*r - r < d2 <= r*r + r
  const int16_t inner = static_cast<int16_t>(span * span - span);
  const int16_t outer = static_cast<int16_t>(span * span + span);

  for (int16_t dy = static_cast<int16_t>(-span); dy <= span; dy++) {
    for (int16_t dx = static_cast<int16_t>(-span); dx <= span; dx++) {
      const int16_t squared = static_cast<int16_t>(dx * dx + dy * dy);

      if (squared <= inner || squared > outer) {
        continue;
      }

      display.setPixel(static_cast<int8_t>(centerX + dx), static_cast<int8_t>(centerY + dy), true);
    }
  }
}
