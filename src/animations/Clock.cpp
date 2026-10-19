#include "animations/Clock.h"

#include "animations/blink.h"
#include "animations/digitFont.h"
#include "animations/marker.h"
#include "network/TimeSync.h"

#include <Arduino.h>
#include <ctime>

const char* Clock::name() const {
  return "clock";
}

void Clock::begin(Display& display) {
  startMs_ = millis();
  lastPollMs_ = startMs_;
  shownMinute_ = kNoMinute;
  blinkLit_ = true;
  synced_ = false;
  markerLit_ = markerLitAt(startMs_);

  if (drawTimeIfChanged(display)) {
    return;
  }

  drawWaiting(display, blinkLit_);
}

void Clock::tick(Display& display, uint32_t nowMs) {
  if (!synced_) {
    const bool lit = blinkLitFirst(nowMs - startMs_, kBlinkPeriodMs, kBlinkOnMs);

    if (lit != blinkLit_) {
      blinkLit_ = lit;
      drawWaiting(display, lit);
    }
  }

  const bool markerNow = markerLitAt(nowMs);

  if (synced_ && markerNow != markerLit_) {
    markerLit_ = markerNow;
    drawTime(display, static_cast<uint8_t>(shownMinute_ / 60),
             static_cast<uint8_t>(shownMinute_ % 60));
  }

  if (nowMs - lastPollMs_ < kPollMs) {
    return;
  }

  lastPollMs_ = nowMs;
  drawTimeIfChanged(display);
}

void Clock::setMarker(bool on) {
  if (on && !marker_) {
    markerStartMs_ = millis();
  }

  marker_ = on;
}

bool Clock::markerLitAt(uint32_t nowMs) const {
  return marker_ && markerLit(nowMs - markerStartMs_);
}

void Clock::drawWaiting(Display& display, bool lit) {
  display.clear();
  display.setPixel(kWaitX, kWaitY, lit);
  display.show();
}

void Clock::drawTime(Display& display, uint8_t hour, uint8_t minute) {
  display.clear();
  drawDigit(display, static_cast<uint8_t>(hour / kTens), kHourTensX, kHourY);
  drawDigit(display, static_cast<uint8_t>(hour % kTens), kHourOnesX, kHourY);
  drawDigit(display, static_cast<uint8_t>(minute / kTens), kMinuteTensX, kMinuteY);
  drawDigit(display, static_cast<uint8_t>(minute % kTens), kMinuteOnesX, kMinuteY);
  display.setPixel(kMarkerX, kMarkerY, markerLit_);
  display.show();
}

bool Clock::drawTimeIfChanged(Display& display) {
  tm local{};

  if (!TimeSync::localTime(local)) {
    return false;
  }

  const int16_t minute = static_cast<int16_t>(local.tm_hour * 60 + local.tm_min);

  if (synced_ && minute == shownMinute_) {
    return true;
  }

  synced_ = true;
  shownMinute_ = minute;
  drawTime(display, static_cast<uint8_t>(local.tm_hour), static_cast<uint8_t>(local.tm_min));
  return true;
}
