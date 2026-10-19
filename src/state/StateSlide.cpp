#include "state/State.h"

#include "board/boardConfig.h"
#include "clockFaces/clockFrame.h"

#include <Arduino.h>

bool StateMachine::slidesInto(const Picture& next) const {
  if (!Board::kHasClockHalf || display_.revealing()) {
    return false;
  }

  const bool fromFace = animation_ == &wideClock_ && !split_;
  const bool toFace = &next.animation == &wideClock_ && !next.split;
  return (fromFace && next.split) || (split_ && toFace);
}

void StateMachine::startSlide(const tm& local) {
  const uint8_t rows = clockFaceRows(settings_.clockFace());
  const uint32_t nowMs = millis();
  slide_.start(display_, split_, rows, settings_.splitClock(), static_cast<uint8_t>(local.tm_hour),
               static_cast<uint8_t>(local.tm_min), nowMs);
  enteredMs_ = nowMs;
}

void StateMachine::completeSlide() {
  if (!slide_.running()) {
    return;
  }

  const bool started = slide_.pictureRuns();
  slide_.stop(display_);

  if (!started) {
    startPicture(false);
  }
}

bool StateMachine::advanceSlide(uint32_t nowMs) {
  if (!slide_.running()) {
    return false;
  }

  // a picture begun after nowMs was sampled must not tick with it, the next loop ticks it
  if (slide_.tick(display_, nowMs) == LayoutSlide::Step::START_PICTURE) {
    startPicture(false);
    return true;
  }

  return pictureHeld();
}

bool StateMachine::pictureHeld() const {
  return slide_.running() && !slide_.pictureRuns();
}
