#include "app/SettingsApplier.h"

#include "board/boardConfig.h"
#include "dimming/BrightnessCeiling.h"
#include "network/TimeSync.h"
#include "overlay/TouchOverlay.h"
#include "power/Power.h"
#include "state/State.h"
#include "touch/FingerTracker.h"
#include "touch/Touch.h"

#include <cstring>

SettingsApplier::SettingsApplier(Settings& settings, StateMachine& state,
                                 BrightnessCeiling& ceiling, Touch& touch, Touch& rightTouch,
                                 FingerTracker& fingers, TouchOverlay& overlay)
    : settings_(settings),
      state_(state),
      ceiling_(ceiling),
      touch_(touch),
      rightTouch_(rightTouch),
      fingers_(fingers),
      overlay_(overlay) {
}

void SettingsApplier::beginTimezone() {
  strncpy(timezoneApplied_, settings_.timezone(), Settings::kTimezoneMax);
  TimeSync::setTimezone(timezoneApplied_);
}

void SettingsApplier::beginPowerSave() {
  powerSaveApplied_ = settings_.powerSave();
  Power::apply(powerSaveApplied_);
}

void SettingsApplier::applyTouch(Touch& coin) {
  coin.setDebug(settings_.debugTouch());
  coin.setTriggerPercent(settings_.touchTrigger());
  coin.setHoverPercent(settings_.touchHover());
}

void SettingsApplier::applyFingers() {
  fingers_.setTouchShare(static_cast<float>(settings_.touchTrigger()) / 100.0f);
  fingers_.setDebug(settings_.debugTouch());
}

void SettingsApplier::apply() {
  ceiling_.refresh();
  state_.applySettings();
  applyTouch(touch_);

  if (Board::kHasClockHalf) {
    applyTouch(rightTouch_);
    applyFingers();
  }

  overlay_.setEnabled(settings_.touchHover() != 0);
  applyTimezone();

  if (settings_.powerSave() != powerSaveApplied_) {
    powerSaveApplied_ = settings_.powerSave();
    Power::apply(powerSaveApplied_);
  }
}

void SettingsApplier::applyTimezone() {
  if (strcmp(timezoneApplied_, settings_.timezone()) == 0) {
    return;
  }

  strncpy(timezoneApplied_, settings_.timezone(), Settings::kTimezoneMax);
  TimeSync::setTimezone(timezoneApplied_);
}
