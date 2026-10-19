// Host definitions of the Display members Display.cpp holds, a push prints the composed canvas.
#include "Display.h"

#include <Arduino.h>
#include <cstdio>
#include <cstring>

namespace {

Frame g_pushed;
uint8_t g_pushedLevel = 0;
bool g_anyPush = false;

/**
 * Prints what the matrices show as "t brightness row0..row7", unless it repeats the last push.
 * @param {const Frame&} frame - composed canvas, bit x of a row is canvas column x
 * @param {uint8_t} level - hardware brightness the canvas shows at
 * @returns {void}
 */
void dumpPush(const Frame& frame, uint8_t level) {
  const bool same = g_anyPush && level == g_pushedLevel &&
                    std::memcmp(frame.rows, g_pushed.rows, sizeof(frame.rows)) == 0;

  if (same) {
    return;
  }

  g_pushed = frame;
  g_pushedLevel = level;
  g_anyPush = true;
  printf("%u %u", millis(), static_cast<unsigned>(level));

  for (uint16_t row : frame.rows) {
    printf(" %u", static_cast<unsigned>(row));
  }

  printf("\n");
}

}

void Display::show() {
  dumpPush(compose(), brightness());
  dirty_ = false;
}

void Display::setBrightness(uint8_t level, uint32_t durationMs) {
  requested_ = level > kMaxBrightness ? kMaxBrightness : level;
  brightness_.rampTo(effectiveBrightness(), durationMs, millis());
}

void Display::setBrightnessNow(uint8_t level) {
  requested_ = level > kMaxBrightness ? kMaxBrightness : level;
  brightness_.rampTo(effectiveBrightness(), 0, millis());
}

uint8_t Display::brightness() const {
  if (writtenLevel_ == kLevelUnset) {
    return requested_;
  }

  return writtenLevel_;
}

void Display::tick(uint32_t nowMs) {
  uint8_t level = 0;

  if (brightness_.tick(nowMs, level)) {
    writtenLevel_ = level;
    // a level write changes the picture without a push, the dump records it as a frame of its own
    dumpPush(g_pushed, level);
  }

  if (dirty_) {
    show();
  }
}

uint8_t Display::effectiveBrightness() const {
  return requested_ > floor_ ? requested_ : floor_;
}
