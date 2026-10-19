#include "Button.h"

#include <Arduino.h>

void Button::begin() {
  pinMode(kPin, INPUT_PULLUP);
  raw_ = digitalRead(kPin) == LOW;
  pressed_ = raw_;
  // a button already down at boot starts its hold here, an unstamped press reads as held since 0
  changedMs_ = millis();
  pressedMs_ = changedMs_;
}

bool Button::tick(uint32_t nowMs) {
  const bool raw = digitalRead(kPin) == LOW;

  if (raw != raw_) {
    raw_ = raw;
    changedMs_ = nowMs;
    return false;
  }

  if (raw == pressed_ || (nowMs - changedMs_) < kDebounceMs) {
    return false;
  }

  pressed_ = raw;

  if (pressed_) {
    pressedMs_ = nowMs;
    return false;
  }

  lastHoldMs_ = nowMs - pressedMs_;
  return true;
}

uint32_t Button::heldMs(uint32_t nowMs) const {
  return pressed_ ? nowMs - pressedMs_ : 0;
}

uint32_t Button::lastHoldMs() const {
  return lastHoldMs_;
}
