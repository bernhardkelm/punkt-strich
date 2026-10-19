#include "state/State.h"

#include <Arduino.h>

void StateMachine::toggleOff(uint32_t nowMs) {
  if (view_ == View::OFF) {
    setView(viewBeforeOff_, nowMs);
    return;
  }

  setView(View::OFF, nowMs);
}

OffReason StateMachine::offReason() const {
  return offReason_;
}

void StateMachine::enterAutoOff(OffReason reason, uint32_t nowMs) {
  if (reason != OffReason::TIMEOUT && reason != OffReason::SCHEDULE) {
    return;
  }

  if (!changeView(View::OFF, nowMs)) {
    return;
  }

  offReason_ = reason;
}

bool StateMachine::leaveAutoOff(uint32_t nowMs) {
  if (offReason_ != OffReason::TIMEOUT && offReason_ != OffReason::SCHEDULE) {
    return false;
  }

  changeView(viewBeforeOff_, nowMs);
  return true;
}

void StateMachine::applyOffTimeout(uint32_t nowMs) {
  const uint16_t minutes = settings_.offTimeoutMin();

  if (minutes == 0 || view_ == View::OFF || !showsClock()) {
    return;
  }

  const uint32_t restingMs = restingSinceMs();
  const uint32_t behind = restingMs - nowMs;

  // a nowMs sampled before the rest started would read as an elapsed eternity after the wrap
  if (behind != 0 && behind < kSampleLagMaxMs) {
    return;
  }

  const uint32_t total = static_cast<uint32_t>(settings_.dimTimeoutMin()) + minutes;

  if (nowMs - restingMs < total * 60000u) {
    return;
  }

  Serial.println("punkt: view off (timeout)");
  enterAutoOff(OffReason::TIMEOUT, nowMs);
}
