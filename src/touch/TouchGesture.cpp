#include "touch/Touch.h"

#include <Arduino.h>

bool Touch::debounce() {
  const bool candidate = touched_ ? !baseline_.atOrBelowPercent(raw_, releasePercent_)
                                  : baseline_.atOrAbovePercent(raw_, triggerPercent_);

  if (candidate == touched_) {
    pending_ = 0;
    return false;
  }

  pending_++;

  if (pending_ < kDebounceSamples) {
    return false;
  }

  pending_ = 0;
  touched_ = candidate;
  return true;
}

TouchEvent Touch::reportDown(uint32_t nowMs) {
  pressedMs_ = nowMs;
  held_ = false;
  stuck_ = false;
  Serial.printf("punkt: %s down raw=%u base=%u\n", name_, raw_, baseline_.value());
  return TouchEvent::NONE;
}

TouchEvent Touch::reportUp(uint32_t nowMs) {
  const uint32_t pressMs = nowMs - pressedMs_;
  Serial.printf("punkt: %s up raw=%u base=%u ms=%u\n", name_, raw_, baseline_.value(), pressMs);

  if (pressMs >= kHoldMaxMs) {
    tapPending_ = false;
    Serial.printf("punkt: %s reset attempt released, no gesture\n", name_);
    return TouchEvent::NONE;
  }

  if (pressMs >= kHoldMs) {
    tapPending_ = false;
    Serial.printf("punkt: %s hold\n", name_);
    return record(TouchEvent::HOLD, nowMs);
  }

  if (tapPending_ && (pressedMs_ - tapReleasedMs_) < kDoubleTapMs) {
    tapPending_ = false;
    Serial.printf("punkt: %s double tap\n", name_);
    return record(TouchEvent::DOUBLE_TAP, nowMs);
  }

  tapPending_ = true;
  tapReleasedMs_ = nowMs;
  Serial.printf("punkt: %s tap\n", name_);
  return record(TouchEvent::TAP, nowMs);
}

TouchEvent Touch::reportTapConfirmed(uint32_t nowMs) {
  if (!tapPending_ || touched_ || (nowMs - tapReleasedMs_) < kDoubleTapMs) {
    return TouchEvent::NONE;
  }

  tapPending_ = false;
  Serial.printf("punkt: %s tap confirmed\n", name_);
  return TouchEvent::TAP_CONFIRMED;
}

void Touch::dropTapOnHold(uint32_t nowMs) {
  if (!touched_ || held_ || (nowMs - pressedMs_) < kHoldMs) {
    return;
  }

  held_ = true;
  tapPending_ = false;
}

TouchEvent Touch::record(TouchEvent event, uint32_t nowMs) {
  lastEvent_ = event;
  lastEventMs_ = nowMs;
  return event;
}
