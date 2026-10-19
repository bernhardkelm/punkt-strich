#include "app/otaProgress.h"

#include "Display.h"
#include "network/Ota.h"
#include "state/State.h"

#include <Arduino.h>

namespace {

constexpr uint32_t kOtaSpiralPixels = Display::kCanvasWidth * Display::kHeight;

StateMachine* otaState = nullptr;

/**
 * Starts the progress spiral at zero while an OTA update runs.
 * @returns {void}
 */
void onOtaStart() {
  const uint32_t nowMs = millis();
  otaState->set(PunktState::LOADING, "ota", nowMs);
  otaState->setOtaProgress(0, nowMs);
}

/**
 * Fills the matrix with the share of the image received and advances the animation while
 * ArduinoOTA.handle() blocks on it.
 * @param {uint32_t} done - bytes received so far
 * @param {uint32_t} total - bytes the image carries, 0 while the library does not know it
 * @returns {void}
 */
void onOtaProgress(uint32_t done, uint32_t total) {
  const uint32_t nowMs = millis();

  if (total > 0) {
    otaState->setOtaProgress(static_cast<int16_t>(done * kOtaSpiralPixels / total), nowMs);
  }

  otaState->tick(nowMs);
}

/**
 * Puts the session picture back after a failed OTA update, the old firmware keeps running.
 * @returns {void}
 */
void onOtaError() {
  otaState->setOtaProgress(-1, millis());
  otaState->endLoading("ota error");
}

}

void beginOta(const char* name, StateMachine& state) {
  otaState = &state;
  Ota::begin(name, onOtaStart, onOtaProgress, onOtaError);
}
