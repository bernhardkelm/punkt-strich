#include "power/NightSleep.h"

#include "network/TimeSync.h"
#include "power/deepSleep.h"
#include "power/nightWindow.h"
#include "power/wakeDelay.h"
#include "settings/Settings.h"
#include "settings/clockMinutes.h"
#include "state/State.h"
#include "touch/Touch.h"

#include <Arduino.h>
#include <ctime>
#include <esp_sleep.h>

namespace {

constexpr uint16_t kMinutesPerHour = 60;
constexpr uint64_t kMicrosPerSecond = 1000000;

uint32_t checkedMs = 0;

}

namespace NightSleep {

bool wokeByTouch() {
  return esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TOUCHPAD &&
         wokeFrom() == SleepKind::NIGHT;
}

bool confirmWake(Touch& touch, Display& display, const Settings& settings) {
  const uint32_t startMs = millis();

  while (millis() - startMs < kWakeConfirmMs) {
    const TouchEvent event = touch.tick(millis());

    if (event != TouchEvent::NONE && event != TouchEvent::HOLD) {
      Serial.println("punkt: wake confirmed by tap");
      return true;
    }

    delay(1);
  }

  Serial.println("punkt: wake not confirmed, sleeping again");
  enter(display, touch, settings);
}

bool due(uint32_t nowMs, const Settings& settings, const StateMachine& state) {
  if (nowMs - checkedMs < kCheckIntervalMs) {
    return false;
  }

  checkedMs = nowMs;

  if (!settings.nightOff() || !settings.nightSleep() || state.loading()) {
    return false;
  }

  if (state.namedSessionCount() > 0) {
    return false;
  }

  tm local = {};

  if (!TimeSync::localTime(local)) {
    return false;
  }

  const uint16_t nowMin = static_cast<uint16_t>(local.tm_hour * kMinutesPerHour + local.tm_min);

  if (!insideNightWindow(nowMin, settings.sleepAt(), settings.wakeAt())) {
    return false;
  }

  return nowMs - state.lastActivityMs() >= kGraceMs;
}

void enter(Display& display, Touch& touch, const Settings& settings) {
  tm local = {};
  char clock[kClockTextSize] = "??:??";
  const bool synced = TimeSync::localTime(local);

  if (synced) {
    const uint16_t nowMin = static_cast<uint16_t>(local.tm_hour * kMinutesPerHour + local.tm_min);
    formatClockMinutes(nowMin, clock, sizeof(clock));
    char wake[kClockTextSize] = {};
    formatClockMinutes(settings.wakeAt(), wake, sizeof(wake));
    esp_sleep_enable_timer_wakeup(static_cast<uint64_t>(secondsUntilWake(local, settings.wakeAt())) *
                                  kMicrosPerSecond);
    Serial.printf("punkt: night sleep at %s, wakes at %s or by double tap\n", clock, wake);
  } else {
    Serial.printf("punkt: night sleep at %s, double tap wakes\n", clock);
  }

  darkenForSleep(display);
  startDeepSleep(SleepKind::NIGHT, touch.pin(), touch.sleepThreshold());
}

}
