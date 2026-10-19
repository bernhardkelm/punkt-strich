#include "touch/FingerTracker.h"

#include "touch/domeLevels.h"

namespace {

constexpr Coin kCoins[2] = {Coin::LEFT, Coin::RIGHT};

/**
 * Returns a sample counter one higher, saturating at 255.
 * @param {uint8_t} count - counter to raise
 * @returns {uint8_t} - raised counter
 */
uint8_t countUp(uint8_t count) {
  return count < 255 ? static_cast<uint8_t>(count + 1) : count;
}

}

void FingerTracker::update(uint32_t nowMs) {
  const bool above = sensor_.strength() >= FingerSensor::kActiveShare;
  nearCount_ = above ? countUp(nearCount_) : 0;
  goneCount_ = above ? 0 : countUp(goneCount_);
  const bool gone = goneCount_ >= kGoneSamples;
  const bool contactBelow = sensor_.strength() < kContactRelease;
  contactGoneCount_ = contactBelow ? countUp(contactGoneCount_) : 0;
  const bool contactGone = contactGoneCount_ >= kContactGoneSamples && press_.singleContact() &&
                           press_.stayed(sensor_.fingerX());

  if (phase_ == Phase::PRESS) {
    trackPress(nowMs, gone || contactGone);
    return;
  }

  if (phase_ == Phase::SWIPE) {
    trackSwipe(nowMs, gone);
    return;
  }

  settle_.update(nowMs);

  if (nearCount_ >= kNearSamples && !settle_.settling()) {
    startPress(nowMs);
  }
}

void FingerTracker::startPress(uint32_t nowMs) {
  press_.begin(sensor_.fingerX(), sensor_.strength(Coin::LEFT), sensor_.strength(Coin::RIGHT),
               nowMs);
  chorded_ = false;
  phase_ = Phase::PRESS;
}

void FingerTracker::trackPress(uint32_t nowMs, bool gone) {
  const float x = sensor_.fingerX();
  press_.track(x, sensor_.strength(Coin::LEFT), sensor_.strength(Coin::RIGHT));
  markChord();

  if (!chorded_ && press_.hold(nowMs)) {
    taps_.drop(press_.coin());
  }

  if (gone) {
    releasePress(nowMs);
    return;
  }

  if (!chorded_ && press_.swipeReady(x)) {
    beginSwipe();
  }
}

void FingerTracker::releasePress(uint32_t nowMs) {
  if (chorded_) {
    logGesture("chord", nowMs);
    endGesture(nowMs, false);
    return;
  }

  if (press_.tap(nowMs)) {
    const FingerGesture gesture = taps_.release(press_.coin(), press_.startMs(), nowMs);
    logGesture(gesture == FingerGesture::DOUBLE_TAP ? "double" : "tap", nowMs);
    emit(gesture, press_.coin());
    // no settle, the second press of a double tap follows within a few samples
    phase_ = Phase::IDLE;
    return;
  }

  if (press_.hold(nowMs)) {
    logGesture("hold", nowMs);
    emit(FingerGesture::HOLD, press_.coin(), false, press_.lengthMs(nowMs));
    endGesture(nowMs, false);
    return;
  }

  logGesture("none", nowMs);
  phase_ = Phase::IDLE;
}

void FingerTracker::beginSwipe() {
  const Coin coin = press_.coin();
  disc_.begin(coin, domeRadius(sensor_.level(coin)));
  taps_.drop(coin);
  phase_ = Phase::SWIPE;
  emit(FingerGesture::SWIPE_BEGIN, coin);
}

void FingerTracker::trackSwipe(uint32_t nowMs, bool gone) {
  const float x = sensor_.fingerX();
  press_.widen(x);
  disc_.follow(x);
  markChord();

  if (chorded_) {
    phase_ = Phase::PRESS;
    emit(FingerGesture::SWIPE_END, disc_.coin());
    return;
  }

  if (!gone) {
    return;
  }

  const bool complete = press_.completes(x);
  logGesture(complete ? "swipe" : "swipe-back", nowMs);
  emit(FingerGesture::SWIPE_END, disc_.coin(), complete);
  endGesture(nowMs, true);
}

void FingerTracker::markChord() {
  if (chorded_ || !chord_.active()) {
    return;
  }

  chorded_ = true;
  taps_.drop(Coin::LEFT);
  taps_.drop(Coin::RIGHT);
}

void FingerTracker::endGesture(uint32_t nowMs, bool hideLevels) {
  phase_ = Phase::IDLE;
  settle_.start(nowMs, hideLevels);
}

void FingerTracker::confirmTaps(uint32_t nowMs) {
  for (const Coin coin : kCoins) {
    const bool pressRunning = phase_ != Phase::IDLE && press_.coin() == coin;

    if (taps_.confirm(coin, pressRunning, nowMs)) {
      emit(FingerGesture::TAP_CONFIRMED, coin);
    }
  }
}
