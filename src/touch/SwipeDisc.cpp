#include "touch/SwipeDisc.h"

#include "touch/fingerPosition.h"

void SwipeDisc::begin(Coin coin, float floor) {
  coin_ = coin;
  floor_ = floor;
  radius_ = floor;
}

void SwipeDisc::follow(float x) {
  radius_ += (radiusAt(x) - radius_) * kFollow;
}

Coin SwipeDisc::coin() const {
  return coin_;
}

float SwipeDisc::radius() const {
  return radius_;
}

float SwipeDisc::floor() const {
  return floor_;
}

float SwipeDisc::radiusAt(float x) const {
  const float centre = coinCentre(coin_);
  const float ahead = coin_ == Coin::LEFT ? x - centre : centre - x;
  const float r = 2.0f * (ahead > 0.0f ? ahead : 0.0f) + 1.0f;
  return r > floor_ ? r : floor_;
}
