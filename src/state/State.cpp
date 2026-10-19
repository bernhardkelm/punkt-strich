#include "state/State.h"

StateMachine::StateMachine(Display& display, Settings& settings)
    : display_(display), settings_(settings), sideClock_(settings), wideClock_(settings) {
}

void StateMachine::begin(uint32_t nowMs) {
  lastActivityMs_ = nowMs;
  ceiling_ = settings_.brightness();
  set(PunktState::LOADING, "boot", nowMs);
}

PunktState StateMachine::current() const {
  return state_;
}

size_t StateMachine::sessions(Session* out, size_t max) const {
  return sessions_.copyTo(out, max);
}

size_t StateMachine::namedSessionCount() const {
  return sessions_.namedCount();
}

bool StateMachine::loading() const {
  return loading_;
}

void StateMachine::tick(uint32_t nowMs) {
  // a nowMs sampled before the state was entered would wrap the animation's elapsed time
  const uint32_t behind = enteredMs_ - nowMs;

  if (behind != 0 && behind < kSampleLagMaxMs) {
    return;
  }

  updateContextPush(nowMs);
  updateUsagePush(nowMs);
  applyLowBattery();

  if (!advanceSlide(nowMs)) {
    if (split_) {
      tickSideClock(nowMs);
    }

    animation_->tick(display_, nowMs);
  }

  stripWipe_.tick(display_, nowMs);
  display_.tick(nowMs);
  settleDone();

  if (sessions_.resumeGlitched(nowMs)) {
    refresh(nullptr, "glitch end");
  }

  expireNoSessionFlash(nowMs);
  expirePreview(nowMs);
  applyClockTimeout(nowMs);
  applyBatteryTimeout(nowMs);
  applyOffTimeout(nowMs);

  const size_t namedBefore = sessions_.namedCount();

  if (!sessions_.expire(nowMs)) {
    return;
  }

  // a watchdog that took the last named session ends like a quit, the board rests on the clock
  if (namedBefore > 0 && sessions_.namedCount() == 0) {
    restAnonymous(nowMs);
  }

  refresh(nullptr, "watchdog");
}

void StateMachine::setBrightness(uint8_t level, uint32_t fadeMs) {
  ceiling_ = level;

  for (Animation* animation : all_) {
    animation->setMaxBrightness(ceiling_);
  }

  loadingOrbit_.setMaxBrightness(ceiling_);
  sideClock_.setMaxBrightness(ceiling_);
  wideClock_.setMaxBrightness(ceiling_);
  idleGauge_.setMaxBrightness(ceiling_);
  batteryFace_.setMaxBrightness(ceiling_);

  // the clock redraws once a minute, the ramp lands the new ceiling without waiting for a frame
  display_.setBrightness(ceiling_, fadeMs);
}

uint8_t StateMachine::brightness() const {
  return ceiling_;
}

void StateMachine::setDimmed(bool dimmed) {
  dimmed_ = dimmed;
}

bool StateMachine::dimmed() const {
  return dimmed_;
}

uint32_t StateMachine::lastActivityMs() const {
  return lastActivityMs_;
}

uint32_t StateMachine::restingSinceMs() const {
  // the later of the two starts the rest, compared as a signed gap so the millis() wrap is safe
  if (static_cast<int32_t>(lastActivityMs_ - enteredMs_) > 0) {
    return lastActivityMs_;
  }

  return enteredMs_;
}
