#include "touch/FingerPress.h"

#include "touch/fingerPosition.h"

#include <cmath>
#include <cstdio>

namespace {

/**
 * Returns the coin with the stronger signal.
 * @param {float} left - strength of the left coin
 * @param {float} right - strength of the right coin
 * @returns {Coin} - RIGHT when it is stronger, LEFT otherwise
 */
Coin strongerCoin(float left, float right) {
  return right > left ? Coin::RIGHT : Coin::LEFT;
}

}

void FingerPress::begin(float x, float left, float right, uint32_t nowMs) {
  startX_ = x;
  startMs_ = nowMs;
  coin_ = strongerCoin(left, right);
  fixed_ = false;
  otherMax_ = 0.0f;
  startMax_ = 0.0f;
  startNow_ = 0.0f;
  pressMax_ = left > right ? left : right;
  minX_ = x;
  maxX_ = x;
}

void FingerPress::track(float x, float left, float right) {
  const float strength = left > right ? left : right;
  pressMax_ = strength > pressMax_ ? strength : pressMax_;

  if (!fixed_ && strength >= kStartShare) {
    fixed_ = true;
    coin_ = strongerCoin(left, right);
    startX_ = x;
    minX_ = x;
    maxX_ = x;
  }

  if (!fixed_) {
    return;
  }

  const float other = coin_ == Coin::LEFT ? right : left;
  otherMax_ = other > otherMax_ ? other : otherMax_;
  startNow_ = coin_ == Coin::LEFT ? left : right;
  startMax_ = startNow_ > startMax_ ? startNow_ : startMax_;
  widen(x);
}

void FingerPress::widen(float x) {
  minX_ = x < minX_ ? x : minX_;
  maxX_ = x > maxX_ ? x : maxX_;
}

bool FingerPress::fixed() const {
  return fixed_;
}

Coin FingerPress::coin() const {
  return coin_;
}

uint32_t FingerPress::startMs() const {
  return startMs_;
}

uint32_t FingerPress::lengthMs(uint32_t nowMs) const {
  return nowMs - startMs_;
}

bool FingerPress::singleContact() const {
  return fixed_ && pressMax_ >= kTapContact && otherMax_ < kOtherShare;
}

bool FingerPress::stayed(float x) const {
  return std::fabs(x - startX_) < kStayColumns;
}

bool FingerPress::tap(uint32_t nowMs) const {
  return singleContact() && lengthMs(nowMs) <= kTapMaxMs;
}

bool FingerPress::hold(uint32_t nowMs) const {
  return singleContact() && lengthMs(nowMs) >= kHoldMs;
}

bool FingerPress::swipeReady(float x) const {
  if (!fixed_ || otherMax_ < kOtherShare || startNow_ >= startMax_ * kHandoverShare) {
    return false;
  }

  const float centre = coinCentre(coin_);
  const bool across = coin_ == Coin::LEFT ? x > centre + kSwipeCross : x < centre - kSwipeCross;
  return across && std::fabs(x - startX_) >= kSwipeTravel;
}

bool FingerPress::completes(float x) const {
  if (coin_ == Coin::LEFT) {
    return x >= kMiddle + kCompletePastMiddle;
  }

  return x <= kMiddle - kCompletePastMiddle;
}

void FingerPress::summarize(char* line, size_t size, const char* kind, uint32_t nowMs) const {
  snprintf(line, size,
           "g,%lu,%s,coin=%u,dur=%lu,start=%.1f,min=%.1f,max=%.1f,smax=%.2f,other=%.2f",
           static_cast<unsigned long>(nowMs), kind, static_cast<unsigned>(coin_),
           static_cast<unsigned long>(lengthMs(nowMs)), startX_, minX_, maxX_, pressMax_,
           otherMax_);
}
