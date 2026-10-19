#include "ResetHold.h"

#include "Display.h"
#include "network/BootCounter.h"
#include "network/WifiSetup.h"
#include "settings/Settings.h"
#include "state/State.h"
#include "touch/Touch.h"

#include <Arduino.h>

static_assert(Touch::kHoldMaxMs == ResetHold::kArmMs,
              "the touch hold window must end where the reset fill arms");

namespace {

constexpr uint16_t kPixelCount = Display::kCanvasWidth * Display::kHeight;

}

ResetHold::ResetHold(StateMachine& state, Settings& settings)
    : state_(state), settings_(settings) {
}

void ResetHold::tick(uint32_t heldMs, uint32_t nowMs) {
  if (erased_) {
    waitForRestart(heldMs, nowMs);
    return;
  }

  if (heldMs < kArmMs) {
    if (armed_) {
      armed_ = false;
      Serial.println("punkt: reset hold released");
      state_.setResetProgress(-1, nowMs);
    }

    return;
  }

  if (!armed_) {
    armed_ = true;
    Serial.println("punkt: reset hold, keep holding to erase settings and wifi");
  }

  if (heldMs >= kResetMs) {
    factoryReset(nowMs);
    return;
  }

  const uint32_t pixels = (heldMs - kArmMs) * kPixelCount / (kResetMs - kArmMs);
  state_.setResetProgress(static_cast<int16_t>(pixels), nowMs);
}

void ResetHold::waitForRestart(uint32_t heldMs, uint32_t nowMs) {
  if (heldMs == 0) {
    Serial.println("punkt: restarting into the setup portal");
    ESP.restart();
    return;
  }

  if ((nowMs - erasedMs_) < kRestartTimeoutMs) {
    return;
  }

  Serial.println("punkt: still held, restarting into the setup portal anyway");
  ESP.restart();
}

void ResetHold::factoryReset(uint32_t nowMs) {
  Serial.println("punkt: factory reset, erasing settings and wifi credentials");
  state_.setResetProgress(kPixelCount, nowMs);
  settings_.reset();
  BootCounter::reset();
  WifiSetup::forget();
  erased_ = true;
  erasedMs_ = nowMs;
  Serial.println("punkt: release the button to restart");
}
