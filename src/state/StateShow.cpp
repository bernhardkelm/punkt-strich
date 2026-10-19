#include "state/State.h"

#include "network/TimeSync.h"
#include "state/stateNames.h"

#include <Arduino.h>
#include <ctime>

namespace {

constexpr const char* kAnonymous = "";

}

void StateMachine::applySettings() {
  if (state_ != PunktState::WORKING) {
    return;
  }

  const int index = sessions_.displayIndex();
  const Session* shown = index < 0 ? nullptr : &sessions_.at(static_cast<size_t>(index));
  const uint8_t activity = shown == nullptr ? kDefaultActivity : shown->activity;
  const int8_t selected = shown == nullptr ? -1 : shown->animation;

  if (animation_ != &displayAnimationFor(PunktState::WORKING, activity, selected).animation) {
    show(PunktState::WORKING, activity, selected, kAnonymous, "settings");
  }
}

void StateMachine::refresh(const char* session, const char* source) {
  // loading owns the matrix, but the reset fill, the link pictures and the OTA spiral replace it
  if (loading_) {
    show(PunktState::LOADING, kDefaultActivity, -1, nullptr, source);
    return;
  }

  const int index = sessions_.displayIndex();

  if (index < 0) {
    show(PunktState::OFF, kDefaultActivity, -1, session, source);
    return;
  }

  const Session& entry = sessions_.at(static_cast<size_t>(index));
  const bool asked = session != nullptr && strncmp(entry.id, session, kSessionIdMax) == 0;
  show(entry.state, entry.activity, entry.animation, asked ? session : nullptr, source);
}

void StateMachine::show(PunktState state, uint8_t activity, int8_t animation, const char* session,
                        const char* source) {
  returnToClockOnIdle(state);
  const Picture next = displayAnimationFor(state, activity, animation);
  applyClockMarker(next.animation, state);
  const bool restart = session != nullptr;

  // tool hooks re-post working every few seconds, a running working animation must survive that
  if (state == state_ && &next.animation == animation_ && next.split == split_ &&
      (!restart || state == PunktState::WORKING)) {
    return;
  }

  if (restart && session[0] != '\0') {
    Serial.printf("punkt: state %s <- %s (%s %s)\n", stateName(state), stateName(state_), source,
                  session);
  } else {
    Serial.printf("punkt: state %s <- %s (%s)\n", stateName(state), stateName(state_), source);
  }

  // a running slide or wipe lands first, the change below starts from the picture it headed to
  completeSlide();
  stripWipe_.stop(display_);
  tm local{};
  const bool slides = slidesInto(next) && TimeSync::localTime(local);
  const bool wipes = wipesInto(next);
  const bool sideClockRuns = split_;
  state_ = state;
  animation_ = &next.animation;
  split_ = next.split;
  display_.setBrightness(ceiling_);

  if (slides) {
    startSlide(local);
    return;
  }

  // the battery face sits left of the clock face on the strip, so it enters from the left edge
  if (wipes) {
    stripWipe_.start(display_, animation_ == &batteryFace_, millis());
  }

  startPicture(sideClockRuns);
}

void StateMachine::startPicture(bool sideClockRuns) {
  // a split-to-split switch leaves the running side clock its half, so it does not flicker
  if (split_ && sideClockRuns) {
    display_.clear();
  } else {
    display_.clearCanvas();
  }

  if (split_ && !sideClockRuns) {
    beginSideClock();
  }

  display_.show();
  animation_->setMaxBrightness(ceiling_);
  animation_->begin(display_);
  enteredMs_ = millis();
}

void StateMachine::applyClockMarker(const Animation& shown, PunktState state) {
  const bool unread = state == PunktState::UNREAD;
  clock_.setMarker(unread && &shown == &clock_);
  wideClock_.setMarker(unread && &shown == &wideClock_);
}

void StateMachine::beginSideClock() {
  display_.setViewportX(0);
  sideClock_.setMaxBrightness(ceiling_);
  sideClock_.begin(display_);
  display_.setViewportX(Display::kViewportX);
}

void StateMachine::tickSideClock(uint32_t nowMs) {
  display_.setViewportX(0);
  sideClock_.tick(display_, nowMs);
  display_.setViewportX(Display::kViewportX);
}
