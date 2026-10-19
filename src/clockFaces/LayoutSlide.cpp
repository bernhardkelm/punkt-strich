#include "clockFaces/LayoutSlide.h"

#include "clockFaces/clockFrame.h"
#include "clockFaces/splitPath.h"

void LayoutSlide::start(Display& display, bool toSplit, uint8_t faceRows, SplitClock style,
                        uint8_t hour, uint8_t minute, uint32_t nowMs) {
  style_ = style;
  hour_ = hour;
  minute_ = minute;
  wipePosition_ = splitPathEnd(style, minute);
  holdsReveal_ = false;
  running_ = true;

  if (toSplit) {
    position_ = fullPosition(faceRows);
    target_ = splitPosition();
    drawClock(display, position_);
  } else {
    position_ = splitPosition();
    target_ = fullPosition(faceRows);
    // the split picture stays on screen while the wipe out reveals the clock drawn behind it
    holdReveal(display);
    drawClock(display, wipePosition_);
  }

  dueMs_ = nowMs + nextStepMs();
}

LayoutSlide::Step LayoutSlide::tick(Display& display, uint32_t nowMs) {
  if (!running_ || static_cast<int32_t>(nowMs - dueMs_) < 0) {
    return Step::NONE;
  }

  const bool toSplit = target_ > position_;
  Step step = Step::NONE;

  if (toSplit) {
    position_++;
    step = stepTowardSplit(display);
  } else {
    position_--;
    step = stepTowardFull(display);
  }

  running_ = position_ != target_;

  if (running_) {
    dueMs_ = nowMs + nextStepMs();
  }

  return step;
}

void LayoutSlide::stop(Display& display) {
  releaseReveal(display);
  running_ = false;
}

bool LayoutSlide::running() const {
  return running_;
}

bool LayoutSlide::pictureRuns() const {
  return running_ && target_ == splitPosition() && position_ >= wipePosition_;
}

uint8_t LayoutSlide::fullPosition(uint8_t faceRows) {
  if (faceRows >= kTallRows) {
    return 0;
  }

  if (faceRows <= kStaircaseRows) {
    return kSquashSteps;
  }

  return static_cast<uint8_t>(kTallRows - faceRows);
}

uint8_t LayoutSlide::splitPosition() const {
  return static_cast<uint8_t>(wipePosition_ + Display::kWidth);
}

Frame LayoutSlide::wipeMask(uint8_t columns, bool splitLive) {
  const uint16_t viewportBits = static_cast<uint16_t>((1U << Display::kWidth) - 1U);
  const uint16_t splitBits = static_cast<uint16_t>((1U << columns) - 1U);
  const uint16_t live = splitLive ? splitBits : static_cast<uint16_t>(viewportBits & ~splitBits);
  const uint16_t bits = static_cast<uint16_t>(live << Display::kViewportX);
  Frame mask;

  for (uint16_t& row : mask.rows) {
    row = bits;
  }

  return mask;
}

void LayoutSlide::drawClock(Display& display, uint8_t position) const {
  drawSplitPath(display, style_, hour_, minute_, position);
  display.show();
}

LayoutSlide::Step LayoutSlide::stepTowardSplit(Display& display) {
  if (position_ < wipePosition_) {
    drawClock(display, position_);
    return Step::NONE;
  }

  if (position_ == wipePosition_) {
    drawClock(display, position_);
    // frozen before the owner starts its picture, which then wipes in behind the clock
    holdReveal(display);
    return Step::START_PICTURE;
  }

  if (position_ == splitPosition()) {
    releaseReveal(display);
    return Step::NONE;
  }

  if (ownsReveal(display)) {
    display.setReveal(wipeMask(static_cast<uint8_t>(position_ - wipePosition_), true));
  }

  return Step::NONE;
}

LayoutSlide::Step LayoutSlide::stepTowardFull(Display& display) {
  if (position_ > wipePosition_) {
    if (ownsReveal(display)) {
      display.setReveal(wipeMask(static_cast<uint8_t>(position_ - wipePosition_), false));
    }

    return Step::NONE;
  }

  if (position_ == wipePosition_) {
    releaseReveal(display);
    return Step::NONE;
  }

  drawClock(display, position_);
  return position_ == target_ ? Step::START_PICTURE : Step::NONE;
}

uint32_t LayoutSlide::nextStepMs() const {
  // the step back out of a position waits as long as the step into it
  const uint8_t step = target_ > position_ ? static_cast<uint8_t>(position_ + 1) : position_;

  if (step > wipePosition_) {
    return kWipeColumnMs;
  }

  return splitPathStepMs(style_, minute_, step);
}

void LayoutSlide::holdReveal(Display& display) {
  display.freeze();
  freezeId_ = display.freezeCount();
  holdsReveal_ = true;
}

bool LayoutSlide::ownsReveal(const Display& display) const {
  return holdsReveal_ && display.revealing() && display.freezeCount() == freezeId_;
}

void LayoutSlide::releaseReveal(Display& display) {
  if (ownsReveal(display)) {
    display.endReveal();
  }

  holdsReveal_ = false;
}
