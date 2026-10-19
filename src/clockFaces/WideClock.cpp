#include "clockFaces/WideClock.h"

#include "animations/blink.h"
#include "animations/marker.h"
#include "clockFaces/clockFrame.h"
#include "network/TimeSync.h"

#include <Arduino.h>
#include <ctime>

WideClock::WideClock(const Settings& settings)
    : settings_(settings) {
}

const char* WideClock::name() const {
  return "clock";
}

void WideClock::begin(Display& display) {
  startMs_ = millis();
  lastPollMs_ = startMs_;
  shownMinute_ = kNoMinute;
  shownFace_ = kNoFace;
  blinkLit_ = true;
  synced_ = false;
  markerLit_ = markerLitAt(startMs_);
  dot_.update(startMs_);

  if (drawTimeIfChanged(display)) {
    return;
  }

  drawWaiting(display, blinkLit_);
}

void WideClock::tick(Display& display, uint32_t nowMs) {
  const bool dotChanged = dot_.update(nowMs);

  if (!synced_) {
    const bool lit = blinkLitFirst(nowMs - startMs_, kBlinkPeriodMs, kBlinkOnMs);

    if (lit != blinkLit_ || dotChanged) {
      blinkLit_ = lit;
      drawWaiting(display, lit);
    }
  }

  const bool markerNow = markerLitAt(nowMs);

  if (synced_ && (markerNow != markerLit_ || dotChanged)) {
    markerLit_ = markerNow;
    drawTime(display, clockFaceRows(settings_.clockFace()),
             static_cast<uint8_t>(shownMinute_ / 60), static_cast<uint8_t>(shownMinute_ % 60));
  }

  if (nowMs - lastPollMs_ < kPollMs) {
    return;
  }

  lastPollMs_ = nowMs;
  drawTimeIfChanged(display);
}

void WideClock::setMarker(bool on) {
  if (on && !marker_) {
    markerStartMs_ = millis();
  }

  marker_ = on;
}

void WideClock::setLowBattery(bool on) {
  dot_.set(on, millis());
}

bool WideClock::markerLitAt(uint32_t nowMs) const {
  return marker_ && markerLit(nowMs - markerStartMs_);
}

void WideClock::drawWaiting(Display& display, bool lit) const {
  display.clearCanvas();
  display.setCanvasPixel(kWaitX, kWaitY, lit);
  drawDot(display);
  display.show();
}

void WideClock::drawDot(Display& display) const {
  if (dot_.active()) {
    display.setCanvasPixel(LowBatteryDot::kX, LowBatteryDot::kY, dot_.lit());
  }
}

void WideClock::drawTime(Display& display, uint8_t rows, uint8_t hour, uint8_t minute) {
  drawClockFrame(display, hour, minute, 0, rows, rows);
  display.setCanvasPixel(kMarkerX, kMarkerY, markerLit_);
  drawDot(display);
  display.show();
}

bool WideClock::drawTimeIfChanged(Display& display) {
  tm local{};

  if (!TimeSync::localTime(local)) {
    return false;
  }

  const int16_t minute = static_cast<int16_t>(local.tm_hour * 60 + local.tm_min);
  const ClockFace face = settings_.clockFace();

  if (synced_ && minute == shownMinute_ && static_cast<int16_t>(face) == shownFace_) {
    return true;
  }

  synced_ = true;
  shownMinute_ = minute;
  shownFace_ = static_cast<int16_t>(face);
  drawTime(display, clockFaceRows(face), static_cast<uint8_t>(local.tm_hour),
           static_cast<uint8_t>(local.tm_min));
  return true;
}
