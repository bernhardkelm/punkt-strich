#include "animations/Pointer.h"

#include "animations/cursorTarget.h"
#include "animations/rectOutline.h"
#include "animations/ripple.h"

#include <Arduino.h>

const char* Pointer::name() const {
  return "pointer";
}

void Pointer::begin(Display& display) {
  x_ = 0;
  y_ = 0;
  originX_ = 0;
  originY_ = 0;
  clickStep_ = 0;
  pickCursorTarget(x_, y_, targetX_, targetY_);
  phase_ = Phase::MOVE;
  nextStepMs_ = millis() + kStepMs;
  draw(display);
}

void Pointer::tick(Display& display, uint32_t nowMs) {
  if (static_cast<int32_t>(nowMs - nextStepMs_) < 0) {
    return;
  }

  if (!advance(nowMs)) {
    return;
  }

  draw(display);
}

void Pointer::moveStep() {
  x_ = static_cast<int8_t>(x_ + stepTowards(x_, targetX_));
  y_ = static_cast<int8_t>(y_ + stepTowards(y_, targetY_));
}

bool Pointer::startAction(uint32_t nowMs) {
  const uint8_t roll = static_cast<uint8_t>(random(100));

  if (roll < kClickPercent) {
    phase_ = Phase::CLICK;
    clickStep_ = 0;
    nextStepMs_ = nowMs + kClickStepMs;
    return true;
  }

  if (roll < kClickPercent + kDragPercent) {
    originX_ = x_;
    originY_ = y_;
    pickDragCorner(x_, y_, targetX_, targetY_);
    phase_ = Phase::DRAG;
    nextStepMs_ = nowMs + kDragStepMs;
    // the rectangle still collapses onto the tip, so the frame equals the one on screen
    return false;
  }

  startRest(nowMs);
  return false;
}

void Pointer::startRest(uint32_t nowMs) {
  phase_ = Phase::REST;
  nextStepMs_ = nowMs + kHoldMs + static_cast<uint32_t>(random(kIdleMaxMs));
}

bool Pointer::advance(uint32_t nowMs) {
  switch (phase_) {
    case Phase::MOVE:
      if (x_ != targetX_ || y_ != targetY_) {
        moveStep();
        nextStepMs_ = nowMs + kStepMs;
        return true;
      }

      return startAction(nowMs);
    case Phase::CLICK:
      clickStep_++;

      if (clickStep_ <= kRippleRings) {
        nextStepMs_ = nowMs + kClickStepMs;
        return true;
      }

      startRest(nowMs);
      // the last ripple step already left the plain L on screen
      return false;
    case Phase::DRAG:
      moveStep();

      if (x_ == targetX_ && y_ == targetY_) {
        phase_ = Phase::DRAG_HOLD;
        nextStepMs_ = nowMs + kDragHoldMs;
        return true;
      }

      nextStepMs_ = nowMs + kDragStepMs;
      return true;
    case Phase::DRAG_HOLD:
      startRest(nowMs);
      return true;
    case Phase::REST:
    default:
      pickCursorTarget(x_, y_, targetX_, targetY_);
      phase_ = Phase::MOVE;
      nextStepMs_ = nowMs + kStepMs;
      return false;
  }
}

void Pointer::draw(Display& display) const {
  display.setBrightness(maxBrightness());
  display.clear();

  if (phase_ == Phase::DRAG || phase_ == Phase::DRAG_HOLD) {
    drawRectOutline(display, originX_, originY_, x_, y_);
  }

  if (phase_ == Phase::CLICK && clickStep_ < kRippleRings) {
    drawRipple(display, x_, y_, static_cast<uint8_t>(clickStep_ + 1));
  }

  drawCursor(display);
  display.show();
}

void Pointer::drawCursor(Display& display) const {
  display.setPixel(x_, y_, true);

  if (phase_ == Phase::CLICK && clickStep_ < kArmsOffSteps) {
    return;
  }

  display.setPixel(static_cast<int8_t>(x_ + 1), y_, true);
  display.setPixel(x_, static_cast<int8_t>(y_ + 1), true);
}
