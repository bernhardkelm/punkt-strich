#include "touch/Touch.h"

#include <Arduino.h>

Touch::Touch(uint8_t pin, const char* name) : name_(name), pin_(pin) {
}

TouchEvent Touch::tick(uint32_t nowMs) {
  if ((nowMs - sampleMs_) < kSampleMs) {
    return TouchEvent::NONE;
  }

  sampleMs_ = nowMs;
  raw_ = touchRead(pin_);
  logDebug(nowMs);

  if (guardFrozen(nowMs)) {
    return TouchEvent::NONE;
  }

  proximity_.update(raw_, baseline_.value(), sleepThreshold(), spread_, hoverPercent_);

  const bool flipped = debounce();

  // the press start must be stamped before trackDrift(), whose stuck check reads it
  if (flipped && touched_) {
    return reportDown(nowMs);
  }

  snapDown();
  trackDrift(nowMs);

  if (!flipped) {
    dropTapOnHold(nowMs);
    return reportTapConfirmed(nowMs);
  }

  return reportUp(nowMs);
}

void Touch::setTriggerPercent(uint8_t percent) {
  triggerPercent_ = 100 + percent;
  releasePercent_ = 100 + static_cast<uint32_t>(percent) * kReleaseShare / 100;
}

void Touch::setHoverPercent(uint8_t percent) {
  hoverPercent_ = percent;
}

uint32_t Touch::sleepThreshold() const {
  const uint64_t delta = static_cast<uint64_t>(baseline_.value()) * (triggerPercent_ - 100) / 100;
  return static_cast<uint32_t>(delta);
}

uint8_t Touch::proximity() const {
  if (hoverPercent_ == 0 || stuck_) {
    return 0;
  }

  if (touched_) {
    return kContactLevel;
  }

  return proximity_.level();
}

bool Touch::tapPending() const {
  return tapPending_;
}

bool Touch::touched() const {
  return touched_;
}

uint8_t Touch::pin() const {
  return pin_;
}

uint32_t Touch::heldMs(uint32_t nowMs) const {
  return touched_ ? nowMs - pressedMs_ : 0;
}

void Touch::setDebug(bool enabled) {
  debug_ = enabled;
}

TouchStatus Touch::status() const {
  return TouchStatus{raw_, baseline_.value(), touched_, lastEvent_,
                     lastEventMs_, restarts_, proximity()};
}

void Touch::logDebug(uint32_t nowMs) {
  if (!debug_ || (nowMs - debugMs_) < kDebugIntervalMs) {
    return;
  }

  debugMs_ = nowMs;
  Serial.printf("punkt: %s raw=%u base=%u touched=%u prox=%u\n", name_, raw_, baseline_.value(),
                touched_ ? 1 : 0, proximity());
}
