#include "state/State.h"

#include "state/stateNames.h"

#include <Arduino.h>
#include <cstring>

namespace {

constexpr const char* kAnonymous = "";
/** source of a request that came over the HTTP API, the only one that pulls the view along */
constexpr const char* kApi = "api";

/**
 * Puts a session into GLITCH and stores the state it interrupted, which resumeGlitched() brings
 * back once kGlitchMs passed. A repeated glitch keeps what the first one stored, a DONE resumes
 * IDLE because that is what it falls back to on its own, and a session this request just added
 * carries the IDLE defaults of resetTo().
 * @param {Session&} entry - session the glitch lands on, still in the state it interrupted
 * @param {uint32_t} nowMs - millis() the watchdog counts from
 * @returns {void}
 */
void enterGlitch(Session& entry, uint32_t nowMs) {
  PunktState resumeState = entry.state;
  uint8_t resumeActivity = entry.activity;
  int8_t resumeAnimation = entry.animation;

  if (entry.state == PunktState::GLITCH) {
    resumeState = entry.resumeState;
    resumeActivity = entry.resumeActivity;
    resumeAnimation = entry.resumeAnimation;
  } else if (entry.state == PunktState::DONE) {
    resumeState = PunktState::IDLE;
    resumeActivity = kDefaultActivity;
    resumeAnimation = -1;
  }

  entry.resetTo(PunktState::GLITCH, nowMs);
  entry.resumeState = resumeState;
  entry.resumeActivity = resumeActivity;
  entry.resumeAnimation = resumeAnimation;
}

}

void StateMachine::set(PunktState state, const char* source, uint32_t nowMs) {
  if (state == PunktState::LOADING) {
    loading_ = true;
    show(state, kDefaultActivity, -1, kAnonymous, source);
    return;
  }

  request(kAnonymous, state, kDefaultActivity, -1, source, nowMs);
}

void StateMachine::setFromApi(const char* session, PunktState state, uint8_t activity,
                              int8_t animation, uint64_t ts, uint32_t nowMs) {
  const Session* entry = sessions_.find(session);

  if (ts != 0 && entry != nullptr && ts < entry->lastTs && entry->lastTs - ts <= kReorderWindowMs) {
    Serial.printf("punkt: stale %s dropped (api %s)\n", stateName(state), session);
    return;
  }

  request(session, state, activity, animation, kApi, nowMs);

  if (ts == 0) {
    return;
  }

  Session* applied = sessions_.find(session);

  if (applied != nullptr) {
    applied->lastTs = ts;
  }
}

void StateMachine::endLoading(const char* source) {
  if (!loading_) {
    return;
  }

  loading_ = false;
  refresh(nullptr, source);
}

void StateMachine::dismissDisplayed(uint32_t nowMs) {
  const int index = sessions_.displayIndex();

  if (index < 0) {
    return;
  }

  request(sessions_.at(static_cast<size_t>(index)).id, PunktState::IDLE, kDefaultActivity, -1,
          "button", nowMs);
}

void StateMachine::request(const char* session, PunktState state, uint8_t activity,
                           int8_t animation, const char* source, uint32_t nowMs) {
  loading_ = false;
  const bool api = strcmp(source, kApi) == 0;

  if (api) {
    lastActivityMs_ = nowMs;
  }

  if (state == PunktState::OFF) {
    sessions_.remove(session);

    if (session[0] != '\0' && sessions_.namedCount() == 0) {
      restAnonymous(nowMs);
      refresh(kAnonymous, "quit");
      return;
    }

    refresh(session, source);
    return;
  }

  // a sign-off leaves the matrix dark, every other request over the API wakes an automatic off
  if (api) {
    leaveAutoOff(nowMs);
  }

  Session* entry = sessions_.find(session);

  if (entry == nullptr) {
    entry = sessions_.add(session, state, nowMs);

    if (entry == nullptr) {
      Serial.printf("punkt: session table full, %s dropped\n", stateName(state));
      return;
    }
  }

  // a state posted over the API is what the user wants to watch, idle and unread keep the clock
  const bool watched = state != PunktState::IDLE && state != PunktState::UNREAD;
  const bool pullable = view_ == View::CLOCK || view_ == View::USAGE || view_ == View::BATTERY;

  if (api && watched && pullable) {
    view_ = View::NORMAL;
    Serial.println("punkt: view normal (api)");
  }

  if (state == PunktState::GLITCH) {
    enterGlitch(*entry, nowMs);
  } else {
    entry->resetTo(state, nowMs);
  }

  if (state == PunktState::WORKING) {
    entry->activity = activity;
    entry->animation = animation;
  }

  refresh(session, source);
}

void StateMachine::restAnonymous(uint32_t nowMs) {
  Session* entry = sessions_.find(kAnonymous);

  if (entry == nullptr) {
    sessions_.add(kAnonymous, PunktState::IDLE, nowMs);
    return;
  }

  entry->resetTo(PunktState::IDLE, nowMs);
}

void StateMachine::settleDone() {
  if (loading_ || state_ != PunktState::DONE) {
    return;
  }

  // wait for a slide to start the picture, a held back done_ still reports its last run
  if (pictureHeld()) {
    return;
  }

  // the one-shot never ticks while the view replaces it, settle it right away instead
  if (animation_ == &animationFor(state_, kDefaultActivity, -1) && !animation_->finished()) {
    return;
  }

  const int index = sessions_.displayIndex();

  if (index < 0) {
    return;
  }

  Session& entry = sessions_.at(static_cast<size_t>(index));

  if (entry.state != PunktState::DONE) {
    return;
  }

  // a finished done means Claude waits for the user, the clock takes over until the next hook
  entry.state = PunktState::IDLE;
  entry.activity = kDefaultActivity;
  entry.animation = -1;
  refresh(nullptr, "done");
}
