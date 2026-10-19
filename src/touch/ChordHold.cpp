#include "touch/ChordHold.h"

void ChordHold::update(float leftStrength, float rightStrength, uint32_t nowMs) {
  if (!active_) {
    const bool each = leftStrength >= kEnterStrength && rightStrength >= kEnterStrength;

    if (each && leftStrength + rightStrength >= kEnterSum) {
      active_ = true;
      startMs_ = nowMs;
    }

    return;
  }

  const float exit = kEnterStrength * kExitShare;
  const bool weak = leftStrength < exit || rightStrength < exit;

  if (weak || leftStrength + rightStrength < kEnterSum * kExitShare) {
    active_ = false;
  }
}

void ChordHold::reset() {
  active_ = false;
}

bool ChordHold::active() const {
  return active_;
}

uint32_t ChordHold::heldMs(uint32_t nowMs) const {
  return active_ ? nowMs - startMs_ : 0;
}
