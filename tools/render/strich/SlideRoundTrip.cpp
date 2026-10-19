#include "SlideRoundTrip.h"

#include "clockFaces/clockFrame.h"
#include "network/TimeSync.h"

#include <ctime>
#include <utility>

SlideRoundTrip::SlideRoundTrip(const Settings& settings, std::unique_ptr<Animation> picture)
    : settings_(settings), face_(settings), split_(settings, std::move(picture)) {
}

const char* SlideRoundTrip::name() const {
  return "slide";
}

void SlideRoundTrip::begin(Display& display) {
  toSplitStarted_ = false;
  toFaceStarted_ = false;
  shown_ = &face_;
  face_.setMaxBrightness(maxBrightness());
  face_.begin(display);
}

void SlideRoundTrip::tick(Display& display, uint32_t nowMs) {
  if (!toSplitStarted_ && nowMs >= kToSplitMs) {
    toSplitStarted_ = true;
    startSlide(display, true, nowMs);
  }

  if (!toFaceStarted_ && nowMs >= kToFaceMs) {
    toFaceStarted_ = true;
    startSlide(display, false, nowMs);
  }

  if (!advanceSlide(display, nowMs)) {
    shown_->tick(display, nowMs);
  }
}

void SlideRoundTrip::startSlide(Display& display, bool toSplit, uint32_t nowMs) {
  tm local{};
  TimeSync::localTime(local);
  shown_ = toSplit ? static_cast<Animation*>(&split_) : &face_;
  slide_.start(display, toSplit, clockFaceRows(settings_.clockFace()), settings_.splitClock(),
               static_cast<uint8_t>(local.tm_hour), static_cast<uint8_t>(local.tm_min), nowMs);
}

bool SlideRoundTrip::advanceSlide(Display& display, uint32_t nowMs) {
  if (!slide_.running()) {
    return false;
  }

  // a picture begun on this tick starts ticking on the next one, like in the state machine
  if (slide_.tick(display, nowMs) == LayoutSlide::Step::START_PICTURE) {
    shown_->setMaxBrightness(maxBrightness());
    shown_->begin(display);
    return true;
  }

  return slide_.running() && !slide_.pictureRuns();
}
