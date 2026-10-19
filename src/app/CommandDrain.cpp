#include "app/CommandDrain.h"

#include "Commands.h"
#include "app/SettingsApplier.h"
#include "app/patchSettings.h"
#include "network/WifiSetup.h"
#include "state/State.h"

#include <Arduino.h>

CommandDrain::CommandDrain(Commands& commands, Settings& settings, StateMachine& state,
                           SettingsApplier& applier)
    : commands_(commands), settings_(settings), state_(state), applier_(applier) {
}

void CommandDrain::drain(uint32_t nowMs) {
  StateRequest request = {};

  while (commands_.takeState(request)) {
    state_.setFromApi(request.session, request.state, request.activity, request.animation,
                      request.ts, nowMs);
  }

  const int16_t context = commands_.contextPercent.exchange(-1);

  if (context >= 0) {
    state_.setContextPercent(static_cast<uint8_t>(context), nowMs);
  }

  const int16_t usage = commands_.usagePercent.exchange(-1);

  if (usage >= 0) {
    state_.setUsagePercent(static_cast<uint8_t>(usage), nowMs);
  }

  applyViewRequest(commands_.viewRequest.exchange(-1), nowMs);

  const int8_t preview = commands_.previewRequest.exchange(-1);

  if (preview >= 0) {
    state_.preview(static_cast<size_t>(preview), nowMs);
  }

  SettingsPatch patch = {};

  while (commands_.takeSettings(patch)) {
    patchSettings(settings_, patch);
    commands_.settingsChanged = true;
  }

  if (commands_.settingsChanged.exchange(false)) {
    applier_.apply();
  }

  if (commands_.forgetWifi.exchange(false)) {
    Serial.println("punkt: wifi forget, restarting");
    WifiSetup::forget();
    armRestart(nowMs);
  }

  if (commands_.reboot.exchange(false)) {
    Serial.println("punkt: reboot");
    armRestart(nowMs);
  }

  if (restartPending_ && nowMs - restartArmedMs_ >= kRestartGraceMs) {
    ESP.restart();
  }
}

void CommandDrain::applyViewRequest(int8_t requested, uint32_t nowMs) {
  if (requested < 0) {
    return;
  }

  if (requested == kViewToggle) {
    state_.toggleView(nowMs);
    return;
  }

  state_.setView(static_cast<View>(requested), nowMs);
}

void CommandDrain::armRestart(uint32_t nowMs) {
  if (restartPending_) {
    return;
  }

  restartPending_ = true;
  restartArmedMs_ = nowMs;
}
