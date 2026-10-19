#include "state/State.h"

#include <Arduino.h>

View StateMachine::view() const {
  return view_;
}

bool StateMachine::viewAllowed(View view) const {
  if (view == view_ || view == View::OFF || view_ == View::OFF) {
    return true;
  }

  const bool toBattery = view == View::BATTERY;
  const bool batteryToClock = view_ == View::BATTERY && view == View::CLOCK;

  // the charge and the clock it returns to say nothing about Claude, they need no session
  if (kHasBatteryView && (toBattery || batteryToClock)) {
    return true;
  }

  return sessions_.namedCount() > 0;
}

View StateMachine::nextView() const {
  switch (view_) {
    case View::CLOCK:
      return View::NORMAL;
    case View::NORMAL:
      return View::USAGE;
    case View::USAGE:
    case View::BATTERY:
      return View::CLOCK;
    case View::OFF:
    default:
      return view_;
  }
}

void StateMachine::setView(View view, uint32_t nowMs) {
  if (!changeView(view, nowMs)) {
    return;
  }

  lastActivityMs_ = nowMs;

  if (view == View::OFF) {
    offReason_ = OffReason::MANUAL;
  }
}

bool StateMachine::changeView(View view, uint32_t nowMs) {
  if (view == view_) {
    return false;
  }

  if (!viewAllowed(view)) {
    Serial.println("punkt: view change refused, no session");
    noSessionFlashUntilMs_ = nowMs + kNoSessionFlashMs;
    refresh(nullptr, "no-session");
    return false;
  }

  if (view == View::OFF) {
    viewBeforeOff_ = view_;
  } else {
    offReason_ = OffReason::NONE;
  }

  view_ = view;
  noSessionFlashUntilMs_ = 0;
  updateContextPush(nowMs);
  updateUsagePush(nowMs);
  refresh(nullptr, "view");
  return true;
}

void StateMachine::toggleView(uint32_t nowMs) {
  setView(nextView(), nowMs);
}

void StateMachine::expireNoSessionFlash(uint32_t nowMs) {
  if (noSessionFlashUntilMs_ == 0 || static_cast<int32_t>(nowMs - noSessionFlashUntilMs_) < 0) {
    return;
  }

  noSessionFlashUntilMs_ = 0;
  refresh(nullptr, "no-session");
}

void StateMachine::applyClockTimeout(uint32_t nowMs) {
  const uint16_t minutes = settings_.clockTimeoutMin();

  if (minutes == 0 || sessions_.namedCount() == 0) {
    return;
  }

  if (view_ != View::NORMAL && view_ != View::USAGE) {
    return;
  }

  const uint32_t behind = lastActivityMs_ - nowMs;

  // a nowMs sampled before the last activity would read as an elapsed eternity after the wrap
  if (behind != 0 && behind < kSampleLagMaxMs) {
    return;
  }

  const uint32_t elapsed = nowMs - lastActivityMs_;

  if (elapsed < static_cast<uint32_t>(minutes) * 60000u) {
    return;
  }

  Serial.println("punkt: view clock (timeout)");
  changeView(View::CLOCK, nowMs);
}

void StateMachine::returnToClockOnIdle(PunktState state) {
  if (view_ != View::NORMAL) {
    return;
  }

  const bool toIdle = state == PunktState::IDLE && state_ != PunktState::IDLE;
  const bool busy = state_ != PunktState::IDLE && state_ != PunktState::UNREAD &&
                    state_ != PunktState::OFF;
  const bool toUnread = state == PunktState::UNREAD && busy;

  if (!toIdle && !toUnread) {
    return;
  }

  // changeView() would re-enter show() through refresh(), the caller reads view_ right after
  view_ = View::CLOCK;
  Serial.println(toIdle ? "punkt: view clock (idle)" : "punkt: view clock (unread)");
}
