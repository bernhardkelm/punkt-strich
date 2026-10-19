#include "dimming/RestDim.h"

#include "settings/Settings.h"
#include "state/State.h"

#include <Arduino.h>

bool RestDim::tick(uint32_t nowMs, const Settings& settings, const StateMachine& state) {
  const bool next = evaluate(nowMs, settings, state);

  if (next == resting_) {
    return false;
  }

  resting_ = next;
  Serial.println(resting_ ? "punkt: rest dim on" : "punkt: rest dim off");
  return true;
}

bool RestDim::resting() const {
  return resting_;
}

bool RestDim::evaluate(uint32_t nowMs, const Settings& settings, const StateMachine& state) const {
  const uint16_t minutes = settings.dimTimeoutMin();

  if (minutes == 0 || !state.showsClock()) {
    return false;
  }

  const uint32_t since = state.restingSinceMs();
  const uint32_t behind = since - nowMs;

  // a nowMs sampled before restingSinceMs would read as an elapsed eternity after the wrap
  if (behind != 0 && behind < kSampleLagMaxMs) {
    return false;
  }

  const uint32_t elapsed = nowMs - since;
  return elapsed >= static_cast<uint32_t>(minutes) * 60000u;
}
