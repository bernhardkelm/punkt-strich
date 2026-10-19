#include "animations/blink.h"

bool blinkLitFirst(uint32_t phase, uint32_t periodMs, uint32_t onMs) {
  return phase % periodMs < onMs;
}

bool blinkLitLast(uint32_t phase, uint32_t periodMs, uint32_t onMs) {
  return phase % periodMs >= periodMs - onMs;
}
