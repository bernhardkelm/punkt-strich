#include "clockFaces/SideClock.h"

#include "animations/blink.h"
#include "clockFaces/canvasFrame.h"
#include "clockFaces/splitClockFrame.h"
#include "clockFaces/squaresRoll.h"
#include "network/TimeSync.h"

#include <Arduino.h>
#include <ctime>

SideClock::SideClock(const Settings& settings)
    : settings_(settings) {
}

const char* SideClock::name() const {
  return "side clock";
}

void SideClock::begin(Display& display) {
  startMs_ = millis();
  lastPollMs_ = startMs_;
  blinkLit_ = true;
  synced_ = false;
  dot_.update(startMs_);
  pollTime(startMs_);
  draw(display);
}

void SideClock::tick(Display& display, uint32_t nowMs) {
  bool changed = dot_.update(nowMs);

  if (!synced_) {
    const bool lit = blinkLitFirst(nowMs - startMs_, kBlinkPeriodMs, kBlinkOnMs);
    changed = changed || lit != blinkLit_;
    blinkLit_ = lit;
  }

  changed = advanceRoll(nowMs) || changed;

  if (nowMs - lastPollMs_ >= kPollMs) {
    lastPollMs_ = nowMs;
    changed = pollTime(nowMs) || changed;
  }

  if (changed) {
    draw(display);
  }
}

void SideClock::setLowBattery(bool on) {
  dot_.set(on, millis());
}

bool SideClock::pollTime(uint32_t nowMs) {
  tm local{};

  if (!TimeSync::localTime(local)) {
    return false;
  }

  const uint16_t minute = static_cast<uint16_t>(local.tm_hour * kMinutesPerHour + local.tm_min);
  const SplitClock style = settings_.splitClock();
  const bool sameStyle = synced_ && style == style_;

  if (sameStyle && minute == targetMinute_) {
    return false;
  }

  targetMinute_ = minute;

  if (sameStyle && style == SplitClock::SQUARES) {
    shownMinute_ = squaresRollStep(shownMinute_, targetMinute_);
    rollDueMs_ = nowMs + kRollStepMs;
  } else {
    shownMinute_ = targetMinute_;
  }

  synced_ = true;
  style_ = style;
  return true;
}

bool SideClock::advanceRoll(uint32_t nowMs) {
  if (shownMinute_ == targetMinute_ || static_cast<int32_t>(nowMs - rollDueMs_) < 0) {
    return false;
  }

  shownMinute_ = squaresRollStep(shownMinute_, targetMinute_);
  rollDueMs_ = nowMs + kRollStepMs;
  return true;
}

void SideClock::draw(Display& display) const {
  if (synced_) {
    const uint8_t hour = static_cast<uint8_t>(shownMinute_ / kMinutesPerHour);
    const uint8_t minute = static_cast<uint8_t>(shownMinute_ % kMinutesPerHour);
    drawViewportFrame(display, splitClockFrame(style_, hour, minute));
  } else {
    display.clear();
    display.setPixel(kWaitX, kWaitY, blinkLit_);
  }

  if (dot_.active()) {
    display.setPixel(LowBatteryDot::kX, LowBatteryDot::kY, dot_.lit());
  }

  display.show();
}
