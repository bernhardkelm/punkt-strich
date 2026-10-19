#include "overlay/TouchOverlay.h"

#include "Display.h"
#include "Frame.h"
#include "board/boardConfig.h"
#include "overlay/discMask.h"

TouchOverlay::TouchOverlay(Display& display)
    : display_(display) {
  clipLast_ = Board::kCanvasWidth - 1;
}

void TouchOverlay::setEnabled(bool enabled) {
  if (enabled_ == enabled) {
    return;
  }

  enabled_ = enabled;

  if (enabled_) {
    return;
  }

  armed_ = false;
  endRipple();
  clearDome();
}

void TouchOverlay::tick(uint8_t leftLevel, uint8_t rightLevel, bool blocked, uint8_t ceiling,
                        uint32_t nowMs) {
  blocked_ = blocked;
  ceiling_ = ceiling;

  if (rippling_ || swipePhase_ != SwipePhase::NONE) {
    if (!enabled_ || blocked_) {
      endRipple();
    } else if (drawReveal(nowMs)) {
      return;
    }
  }

  drawDome(leftLevel, rightLevel);
}

void TouchOverlay::drawDome(uint8_t leftLevel, uint8_t rightLevel) {
  if (!enabled_ || blocked_ || (leftLevel == 0 && rightLevel == 0)) {
    clearDome();
    return;
  }

  Frame on;
  Frame off;
  const uint8_t leftIndex = leftLevel < kLevels ? leftLevel : kLevels - 1;
  const uint8_t rightIndex = rightLevel < kLevels ? rightLevel : kLevels - 1;

  if (leftLevel > 0) {
    const Frame dome = discMask(kLevelR2[leftIndex], 0, 0, kHalfWidth - 1);
    on = merged(on, dome);
    off = merged(off, keyline(dome, 0, kHalfWidth - 1));
  }

  if (rightLevel > 0 && Board::kCanvasWidth > kHalfWidth) {
    const Frame dome = discMask(kLevelR2[rightIndex], kHalfWidth, kHalfWidth, 2 * kHalfWidth - 1);
    on = merged(on, dome);
    off = merged(off, keyline(dome, kHalfWidth, 2 * kHalfWidth - 1));
  }

  display_.setOverlay(on, off);
  display_.setBrightnessFloor(domeFloor());
}

void TouchOverlay::clearDome() {
  display_.clearOverlay();
  display_.setBrightnessFloor(0);
}

uint8_t TouchOverlay::domeFloor() const {
  return ceiling_ < kDomeFloor ? ceiling_ : kDomeFloor;
}
