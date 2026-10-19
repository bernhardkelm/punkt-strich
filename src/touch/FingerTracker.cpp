#include "touch/FingerTracker.h"

FingerTracker::FingerTracker(uint8_t leftPin, uint8_t rightPin) : sensor_(leftPin, rightPin) {
}

void FingerTracker::begin(uint32_t nowMs) {
  sensor_.begin();
  lastSampleMs_ = nowMs;
  phase_ = Phase::IDLE;
  nearCount_ = 0;
  goneCount_ = 0;
  settle_.reset();
  chorded_ = false;
}

void FingerTracker::powerChanged(uint32_t nowMs) {
  rebase(nowMs);
  rebaseAtMs_ = nowMs + kRebaseFollowUpMs;

  if (rebaseAtMs_ == 0) {
    rebaseAtMs_ = 1;
  }
}

void FingerTracker::rebase(uint32_t nowMs) {
  sensor_.rebase();
  chord_.reset();
  chorded_ = false;
  taps_.drop(Coin::LEFT);
  taps_.drop(Coin::RIGHT);
  nearCount_ = 0;
  goneCount_ = 0;
  contactGoneCount_ = 0;

  if (phase_ == Phase::SWIPE) {
    emit(FingerGesture::SWIPE_END, disc_.coin());
  }

  phase_ = Phase::IDLE;
  settle_.start(nowMs, true);
}

void FingerTracker::setTouchShare(float share) {
  sensor_.setTouchShare(share);
}

void FingerTracker::setDebug(bool enabled) {
  debug_ = enabled;
}

FingerEvent FingerTracker::tick(uint32_t nowMs) {
  if (rebaseAtMs_ != 0 && static_cast<int32_t>(nowMs - rebaseAtMs_) >= 0) {
    rebaseAtMs_ = 0;
    rebase(nowMs);
  }

  if ((nowMs - lastSampleMs_) >= FingerSensor::kSampleMs) {
    lastSampleMs_ = nowMs;
    sample(nowMs);
  }

  return events_.pop();
}

float FingerTracker::fingerX() const {
  return sensor_.fingerX();
}

uint8_t FingerTracker::level(Coin coin) const {
  if (phase_ == Phase::SWIPE || settle_.levelsHidden()) {
    return 0;
  }

  if (phase_ == Phase::PRESS && press_.fixed() && !chorded_ && coin != press_.coin()) {
    return 0;
  }

  return sensor_.level(coin);
}

bool FingerTracker::swiping() const {
  return phase_ == Phase::SWIPE;
}

float FingerTracker::swipeRadius() const {
  return disc_.radius();
}

float FingerTracker::swipeFloor() const {
  return disc_.floor();
}

uint32_t FingerTracker::bothHeldMs(uint32_t nowMs) const {
  return chord_.heldMs(nowMs);
}

void FingerTracker::sample(uint32_t nowMs) {
  const bool idle = phase_ == Phase::IDLE && sensor_.strength() < FingerSensor::kActiveShare;
  sensor_.sample(idle);
  chord_.update(sensor_.strength(Coin::LEFT), sensor_.strength(Coin::RIGHT), nowMs);
  // confirmations first, a press starting in this sample must not hold back an expired window
  confirmTaps(nowMs);
  update(nowMs);
  logSample(nowMs);
}

void FingerTracker::emit(FingerGesture gesture, Coin coin, bool complete, uint32_t heldMs) {
  FingerEvent event;
  event.gesture = gesture;
  event.coin = coin;
  event.complete = complete;
  event.heldMs = heldMs;
  events_.push(event);
}

void FingerTracker::logGesture(const char* kind, uint32_t nowMs) {
  if (!debug_) {
    return;
  }

  char line[GestureLog::kLineSize];
  press_.summarize(line, sizeof(line), kind, nowMs);
  log_.record(line);
}

void FingerTracker::logSample(uint32_t nowMs) {
  if (!debug_) {
    return;
  }

  if (phase_ == Phase::IDLE) {
    log_.repeat(nowMs);
  }

  if (!sensor_.resting() || phase_ != Phase::IDLE) {
    sensor_.logSample(nowMs, static_cast<uint8_t>(phase_), disc_.radius());
    return;
  }

  sensor_.logIdle(nowMs);
}
