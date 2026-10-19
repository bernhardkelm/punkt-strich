#include "Commands.h"

#include <cstring>

Commands::Commands()
  : stateQueue_(xQueueCreate(kStateQueueLength, sizeof(StateRequest))),
    settingsQueue_(xQueueCreate(kSettingsQueueLength, sizeof(SettingsPatch))) {
}

bool Commands::postState(const char* session, PunktState state, uint8_t activity, int8_t animation,
                         uint64_t ts) {
  if (stateQueue_ == nullptr || session == nullptr) {
    return false;
  }

  StateRequest request = {};
  strncpy(request.session, session, kSessionIdMax);
  request.state = state;
  request.activity = activity;
  request.animation = animation;
  request.ts = ts;
  return xQueueSend(stateQueue_, &request, 0) == pdTRUE;
}

bool Commands::takeState(StateRequest& out) {
  if (stateQueue_ == nullptr) {
    return false;
  }

  return xQueueReceive(stateQueue_, &out, 0) == pdTRUE;
}

bool Commands::postSettings(const SettingsPatch& patch) {
  if (settingsQueue_ == nullptr) {
    return false;
  }

  return xQueueSend(settingsQueue_, &patch, 0) == pdTRUE;
}

bool Commands::takeSettings(SettingsPatch& out) {
  if (settingsQueue_ == nullptr) {
    return false;
  }

  return xQueueReceive(settingsQueue_, &out, 0) == pdTRUE;
}
