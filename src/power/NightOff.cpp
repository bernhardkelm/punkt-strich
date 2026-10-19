#include "power/NightOff.h"

#include "network/TimeSync.h"
#include "power/NightSleep.h"
#include "power/nightWindow.h"
#include "settings/Settings.h"
#include "state/State.h"

#include <Arduino.h>
#include <ctime>

namespace {

constexpr uint16_t kMinutesPerHour = 60;

uint32_t checkedMs = 0;

}

namespace NightOff {

void tick(uint32_t nowMs, const Settings& settings, StateMachine& state) {
  if (nowMs - checkedMs < kCheckIntervalMs) {
    return;
  }

  checkedMs = nowMs;

  tm local = {};

  if (!TimeSync::localTime(local)) {
    return;
  }

  const uint16_t nowMin = static_cast<uint16_t>(local.tm_hour * kMinutesPerHour + local.tm_min);
  const bool inside =
      settings.nightOff() && insideNightWindow(nowMin, settings.sleepAt(), settings.wakeAt());

  if (inside) {
    if (state.offReason() == OffReason::NONE &&
        nowMs - state.lastActivityMs() >= NightSleep::kGraceMs) {
      state.enterAutoOff(OffReason::SCHEDULE, nowMs);
      Serial.println("punkt: view off (night)");
    }

    return;
  }

  if (state.offReason() == OffReason::SCHEDULE) {
    state.leaveAutoOff(nowMs);
    Serial.println("punkt: view on (night over)");
  }
}

}
