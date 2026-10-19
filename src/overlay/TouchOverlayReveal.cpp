#include "overlay/TouchOverlay.h"

#include "Display.h"
#include "Frame.h"
#include "board/boardConfig.h"
#include "overlay/discMask.h"

#include <cmath>

namespace {

/**
 * Returns the left column of a coin's half.
 * @param {Coin} coin - coin to look up
 * @param {uint8_t} halfWidth - columns per half
 * @returns {uint8_t} - first column of the half
 */
uint8_t halfStart(Coin coin, uint8_t halfWidth) {
  return coin == Coin::RIGHT ? halfWidth : 0;
}

/**
 * Eases a linear progress share into a fast start that slows toward the end, so the ripple front
 * does not stall on the bottom row the way a plain quadratic ease-out would. The front starts at
 * 1.5 times the average speed and eases down to half the average speed by the end.
 * @param {float} t - linear progress from 0 to 1
 * @returns {float} - eased progress from 0 to 1
 */
float eased(float t) {
  return t * (1.5f - 0.5f * t);
}

}

void TouchOverlay::gestureBegins() {
  if (!enabled_ || blocked_) {
    return;
  }

  const uint8_t floor = domeFloor();
  const uint8_t frozen = display_.brightness();
  // floor set before the picture switches, the new animation's brightness would dip the old frame
  rippleFloor_ = frozen > floor ? frozen : floor;
  display_.freeze();
  display_.setBrightnessFloor(rippleFloor_);
  swipePhase_ = SwipePhase::NONE;
  armed_ = true;
}

void TouchOverlay::gestureEnds(bool pictureChanged, Coin coin, RippleScope scope,
                               uint32_t nowMs) {
  if (!armed_) {
    return;
  }

  armed_ = false;

  if (!pictureChanged) {
    rippling_ = false;
    display_.endReveal();
    return;
  }

  originX_ = halfStart(coin, kHalfWidth);

  if (scope == RippleScope::HALF) {
    clipFirst_ = originX_;
    clipLast_ = originX_ + kHalfWidth - 1;
  } else {
    clipFirst_ = 0;
    clipLast_ = Board::kCanvasWidth - 1;
  }

  startRipple(kStartRadius, cornerRadius(originX_, clipFirst_, clipLast_) + kFrontWidth, nowMs);
  drawRipple(nowMs);
}

bool TouchOverlay::swipeBegins(Coin coin, float floorRadius) {
  gestureBegins();

  if (!armed_) {
    return false;
  }

  armed_ = false;
  rippling_ = false;
  originX_ = halfStart(coin, kHalfWidth);
  clipFirst_ = 0;
  clipLast_ = Board::kCanvasWidth - 1;
  swipeFloor_ = floorRadius;
  swipeRadius_ = floorRadius;
  swipePhase_ = SwipePhase::FOLLOW;
  return true;
}

void TouchOverlay::swipeTo(float radius) {
  if (swipePhase_ != SwipePhase::FOLLOW) {
    return;
  }

  swipeRadius_ = radius > swipeFloor_ ? radius : swipeFloor_;
}

void TouchOverlay::swipeEnds(bool complete, uint32_t nowMs) {
  if (swipePhase_ != SwipePhase::FOLLOW) {
    return;
  }

  const float corner = cornerRadius(originX_, clipFirst_, clipLast_) + kFrontWidth;
  const float farthest = corner > swipeRadius_ ? corner : swipeRadius_;
  startRipple(swipeRadius_, complete ? farthest : swipeFloor_, nowMs);
  swipePhase_ = complete ? SwipePhase::NONE : SwipePhase::RETRACT;
  drawRipple(nowMs);
}

bool TouchOverlay::retracting() const {
  return swipePhase_ == SwipePhase::FOLLOW || swipePhase_ == SwipePhase::RETRACT;
}

void TouchOverlay::endSwipe() {
  if (swipePhase_ == SwipePhase::NONE) {
    return;
  }

  endRipple();
}

void TouchOverlay::startRipple(float from, float to, uint32_t nowMs) {
  const float referenceEnd = cornerRadius(0, 0, kReferenceWidth - 1) + kFrontWidth;
  const float travelRatio = std::fabs(to - from) / (referenceEnd - kStartRadius);
  rippleFrom_ = from;
  rippleTo_ = to;
  rippleMs_ = static_cast<uint32_t>(static_cast<float>(kReferenceRippleMs) * travelRatio + 0.5f);
  rippling_ = true;
  rippleStartMs_ = nowMs;
}

bool TouchOverlay::drawReveal(uint32_t nowMs) {
  if (rippling_) {
    return drawRipple(nowMs);
  }

  drawDisc(swipeRadius_);
  return true;
}

bool TouchOverlay::drawRipple(uint32_t nowMs) {
  const uint32_t elapsedMs = nowMs - rippleStartMs_;

  if (elapsedMs >= rippleMs_) {
    if (swipePhase_ != SwipePhase::RETRACT) {
      endRipple();
      return false;
    }

    rippling_ = false;
    swipeRadius_ = rippleTo_;
    swipePhase_ = SwipePhase::HELD;
    drawDisc(swipeRadius_);
    return true;
  }

  const float t = static_cast<float>(elapsedMs) / static_cast<float>(rippleMs_);
  const float progress = eased(t);
  drawDisc(rippleFrom_ + (rippleTo_ - rippleFrom_) * progress);
  return true;
}

void TouchOverlay::drawDisc(float radius) {
  const float innerRadius = radius > kFrontWidth ? radius - kFrontWidth : 0.0f;
  const uint16_t outerR2 = squaredRadius(radius);
  const uint16_t innerR2 = squaredRadius(innerRadius);

  display_.setBrightnessFloor(rippleFloor_);
  display_.setReveal(discMask(outerR2, originX_, clipFirst_, clipLast_));
  display_.setOverlay(ringMask(outerR2, innerR2, originX_, clipFirst_, clipLast_), Frame{});
}

void TouchOverlay::endRipple() {
  rippling_ = false;
  swipePhase_ = SwipePhase::NONE;
  display_.endReveal();
  display_.clearOverlay();
}
