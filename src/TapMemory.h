#pragma once

#include "state/State.h"

#include <cstdint>

/** What the last TAP did, so the DOUBLE_TAP or TAP_CONFIRMED that follows can finish it. */
enum class TapEffect : uint8_t { NONE, SWITCHED_VIEW, WOKE, ACK_WAITS };

struct TapMemory {
  TapEffect effect = TapEffect::NONE;
  /** view the tap left, restored before a double tap turns the matrix off */
  View viewBefore = View::CLOCK;
};
