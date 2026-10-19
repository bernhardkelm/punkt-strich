#include "state/State.h"

namespace {

constexpr const char* kAnonymous = "";
constexpr const char* kTouch = "touch";

}

void StateMachine::acknowledgeDisplayed(uint32_t nowMs) {
  const int index = sessions_.displayIndex();

  if (index < 0) {
    return;
  }

  Session& entry = sessions_.at(static_cast<size_t>(index));

  if (entry.state != PunktState::ATTENTION) {
    return;
  }

  request(entry.id, PunktState::WORKING, kDefaultActivity, -1, kTouch, nowMs);
}

void StateMachine::forceIdle(uint32_t nowMs) {
  loading_ = false;

  if (sessions_.count() == 0) {
    restAnonymous(nowMs);
    refresh(kAnonymous, kTouch);
    return;
  }

  sessions_.idleAll(nowMs);
  refresh(nullptr, kTouch);
}

void StateMachine::noteActivity(uint32_t nowMs) {
  sessions_.touchAll(nowMs);
  lastActivityMs_ = nowMs;
}
