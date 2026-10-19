#pragma once

#include <cstdint>

/** Gesture recognised on the capacitive electrode. */
enum class TouchEvent : uint8_t {
  NONE,
  TAP,
  HOLD,
  DOUBLE_TAP,
  TAP_CONFIRMED,
};
