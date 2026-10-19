#include "power/BatterySleep.h"

#include "board/boardConfig.h"
#include "power/VbusSense.h"
#include "power/deepSleep.h"
#include "settings/Settings.h"
#include "state/SessionTable.h"
#include "state/State.h"
#include "touch/Touch.h"

#include <Arduino.h>
#include <esp_sleep.h>

namespace {

constexpr uint32_t kMsPerMinute = 60000;
constexpr uint64_t kMicrosPerMs = 1000;
/** a last activity up to this far ahead of nowMs was stamped later in the same loop() pass */
constexpr uint32_t kSampleLagMaxMs = 1000;

uint32_t checkedMs = 0;
/** millis() of the last due() call that saw USB power, unplugging counts as input */
uint32_t usbSeenMs = 0;

/**
 * Returns whether a session state keeps the board awake on battery: work in progress, a state
 * waiting for a human, or a result not seen yet.
 * @param {PunktState} state - state of one session
 * @returns {bool} - true for WORKING, ATTENTION, ERROR, WAITING, COMPACTING, LIMIT, UNREAD and DONE
 */
bool holdsAwake(PunktState state) {
  switch (state) {
    case PunktState::WORKING:
    case PunktState::ATTENTION:
    case PunktState::ERROR:
    case PunktState::WAITING:
    case PunktState::COMPACTING:
    case PunktState::LIMIT:
    case PunktState::UNREAD:
    case PunktState::DONE:
      return true;
    default:
      return false;
  }
}

/**
 * Returns whether any session holds the board awake, a glitching session by the state it
 * returns to.
 * @param {const StateMachine&} state - state machine the session table is copied from
 * @returns {bool} - true while one session is in a state holdsAwake() accepts
 */
bool sessionsHoldAwake(const StateMachine& state) {
  Session copies[SessionTable::kMaxSessions];
  const size_t count = state.sessions(copies, SessionTable::kMaxSessions);

  for (size_t i = 0; i < count; i++) {
    const Session& session = copies[i];
    const bool glitch = session.state == PunktState::GLITCH;

    if (holdsAwake(glitch ? session.resumeState : session.state)) {
      return true;
    }
  }

  return false;
}

/**
 * Returns the later of two millis() stamps, compared as a signed gap so the wrap is safe.
 * @param {uint32_t} a - first stamp
 * @param {uint32_t} b - second stamp
 * @returns {uint32_t} - the stamp that lies later
 */
uint32_t later(uint32_t a, uint32_t b) {
  return static_cast<int32_t>(a - b) > 0 ? a : b;
}

/**
 * Returns how long the last input lies back, 0 when it was stamped after nowMs was sampled.
 * @param {uint32_t} nowMs - millis() at the time of the call
 * @param {uint32_t} lastMs - millis() of the last input
 * @returns {uint32_t} - milliseconds without input
 */
uint32_t idleMs(uint32_t nowMs, uint32_t lastMs) {
  if (lastMs - nowMs < kSampleLagMaxMs) {
    return 0;
  }

  return nowMs - lastMs;
}

/**
 * Returns whether the cell is close to empty.
 * @returns {bool} - true while the estimate reads kCriticalPercent or less
 */
bool critical() {
  const int percent = VbusSense::percent();
  return percent >= 0 && percent <= BatterySleep::kCriticalPercent;
}

/**
 * Arms the timer wake that looks for USB power.
 * @returns {void}
 */
void armUsbCheck() {
  esp_sleep_enable_timer_wakeup(static_cast<uint64_t>(BatterySleep::kUsbCheckMs) * kMicrosPerMs);
}

}

namespace BatterySleep {

void recheckUsb() {
  if (!Board::kHasVbusSense || esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_TIMER) {
    return;
  }

  if (wokeFrom() != SleepKind::BATTERY) {
    return;
  }

  VbusSense::begin();
  VbusSense::tick(millis());

  if (VbusSense::usbPresent()) {
    Serial.println("punkt: usb back, leaving the battery sleep");
    return;
  }

  Serial.println("punkt: still on battery, sleeping again");
  armUsbCheck();
  startDeepSleep(SleepKind::BATTERY, Touch::kLeftPin, storedWakeThreshold());
}

bool due(uint32_t nowMs, const Settings& settings, const StateMachine& state) {
  if (!Board::kHasVbusSense) {
    return false;
  }

  if (VbusSense::usbPresent()) {
    usbSeenMs = nowMs;
    return false;
  }

  if (nowMs - checkedMs < kCheckIntervalMs) {
    return false;
  }

  checkedMs = nowMs;

  const uint32_t idle = idleMs(nowMs, later(state.lastActivityMs(), usbSeenMs));

  if (critical() && idle >= kCriticalGraceMs) {
    return true;
  }

  const uint16_t minutes = settings.batterySleepMin();

  if (minutes == 0 || sessionsHoldAwake(state)) {
    return false;
  }

  return idle >= static_cast<uint32_t>(minutes) * kMsPerMinute;
}

void enter(Display& display, Touch& touch) {
  Serial.printf("punkt: battery sleep at %d percent, left coin or usb wakes\n",
                VbusSense::percent());
  armUsbCheck();
  darkenForSleep(display);
  startDeepSleep(SleepKind::BATTERY, touch.pin(), touch.sleepThreshold());
}

}
