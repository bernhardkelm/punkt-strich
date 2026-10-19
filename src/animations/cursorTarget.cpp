#include "animations/cursorTarget.h"

#include "Display.h"

#include <Arduino.h>

namespace {

/** the L spans two columns and two rows, a tip has to leave room for both arms */
constexpr int8_t kArmSpan = 2;
constexpr int8_t kMaxTipX = Display::kWidth - kArmSpan;
constexpr int8_t kMaxTipY = Display::kHeight - kArmSpan;
constexpr uint8_t kTargetRangeX = kMaxTipX + 1;
constexpr uint8_t kTargetRangeY = kMaxTipY + 1;
constexpr uint8_t kTargetTries = 4;
constexpr int8_t kDragMinDelta = 2;
constexpr uint8_t kDragTries = 30;

/**
 * Returns the coordinate a failed corner draw falls back to, the nearest one the L still fits on.
 * @param {int8_t} from - arrival coordinate
 * @param {int8_t} max - highest coordinate the L fits on
 * @returns {int8_t} - coordinate at least kDragMinDelta away from the arrival
 */
int8_t fallbackCorner(int8_t from, int8_t max) {
  if (from + kDragMinDelta <= max) {
    return static_cast<int8_t>(from + kDragMinDelta);
  }

  return static_cast<int8_t>(from - kDragMinDelta);
}

/**
 * Returns whether two coordinates lie far enough apart for a drag rectangle.
 * @param {int8_t} from - arrival coordinate
 * @param {int8_t} to - candidate coordinate
 * @returns {bool} - true at kDragMinDelta or more in either direction
 */
bool farEnough(int8_t from, int8_t to) {
  const int8_t delta = static_cast<int8_t>(to - from);

  return delta >= kDragMinDelta || delta <= -kDragMinDelta;
}

}  // namespace

int8_t stepTowards(int8_t from, int8_t to) {
  if (from < to) {
    return 1;
  }

  if (from > to) {
    return -1;
  }

  return 0;
}

void pickCursorTarget(int8_t x, int8_t y, int8_t& targetX, int8_t& targetY) {
  for (uint8_t i = 0; i < kTargetTries; i++) {
    targetX = static_cast<int8_t>(random(kTargetRangeX));
    targetY = static_cast<int8_t>(random(kTargetRangeY));

    if (targetX != x || targetY != y) {
      return;
    }
  }
}

void pickDragCorner(int8_t x, int8_t y, int8_t& cornerX, int8_t& cornerY) {
  for (uint8_t i = 0; i < kDragTries; i++) {
    const int8_t candidateX = static_cast<int8_t>(random(kTargetRangeX));
    const int8_t candidateY = static_cast<int8_t>(random(kTargetRangeY));

    if (farEnough(x, candidateX) && farEnough(y, candidateY)) {
      cornerX = candidateX;
      cornerY = candidateY;
      return;
    }
  }

  cornerX = fallbackCorner(x, kMaxTipX);
  cornerY = fallbackCorner(y, kMaxTipY);
}
