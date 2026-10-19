#include "touch/GestureSettle.h"

void GestureSettle::start(uint32_t nowMs, bool hideLevels) {
  startMs_ = nowMs;
  settling_ = true;
  levelsHidden_ = hideLevels;
}

void GestureSettle::update(uint32_t nowMs) {
  if (!settling_ || (nowMs - startMs_) < kSettleMs) {
    return;
  }

  reset();
}

void GestureSettle::reset() {
  settling_ = false;
  levelsHidden_ = false;
}

bool GestureSettle::settling() const {
  return settling_;
}

bool GestureSettle::levelsHidden() const {
  return levelsHidden_;
}
