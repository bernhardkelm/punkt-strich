#pragma once

#include "touch/Coin.h"

#include <cstdint>

/** Gesture the finger tracker reports, SWIPE_BEGIN and SWIPE_END frame a running swipe. */
enum class FingerGesture : uint8_t {
  NONE,
  TAP,
  DOUBLE_TAP,
  TAP_CONFIRMED,
  HOLD,
  SWIPE_BEGIN,
  SWIPE_END,
};

/** One gesture of the finger tracker and the coin it started on. */
struct FingerEvent {
  FingerGesture gesture = FingerGesture::NONE;
  /** coin the gesture started on */
  Coin coin = Coin::LEFT;
  /** SWIPE_END only, the finger was released past the canvas middle */
  bool complete = false;
  /** HOLD only, contact length of the released single-coin hold */
  uint32_t heldMs = 0;
};
