#include "web/settingsRoutes.h"

#include "Commands.h"
#include "settings/Settings.h"
#include "web/apiCommon.h"
#include "web/patchDetail.h"
#include "web/settingsDoc.h"
#include "web/settingsPatch.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>
#include <ESPAsyncWebServer.h>

namespace {

Commands* commandQueue = nullptr;
Settings* settingsStore = nullptr;

/**
 * Answers GET /settings with the full settings object.
 * @param {AsyncWebServerRequest*} request - request to answer
 * @returns {void}
 */
void handleGet(AsyncWebServerRequest* request) {
  const SettingsPatch none = {};
  sendApiJson(request, settingsDoc(*settingsStore, none), "");
}

/**
 * Validates the body and hands the patch to loop(), which owns the NVS writes. The response
 * already reports the patched settings, no handler waits for a flash commit.
 * @param {AsyncWebServerRequest*} request - request to answer
 * @param {JsonVariant&} json - parsed request body
 * @returns {void}
 */
void handlePost(AsyncWebServerRequest* request, JsonVariant& json) {
  SettingsPatch patch = {};
  const char* error = nullptr;

  if (!readSettingsPatch(json, patch, error)) {
    sendApiError(request, error);
    return;
  }

  if (!commandQueue->postSettings(patch)) {
    sendApiError(request, kQueueFullError, 503);
    return;
  }

  const String detail = patchDetail(patch);
  sendApiJson(request, settingsDoc(*settingsStore, patch), detail.c_str());
}

}

void registerSettingsRoutes(AsyncWebServer& server, Commands& commands, Settings& settings) {
  commandQueue = &commands;
  settingsStore = &settings;

  server.on(AsyncURIMatcher::exact("/settings"), HTTP_GET, handleGet);

  auto* post = new AsyncCallbackJsonWebHandler(AsyncURIMatcher::exact("/settings"), handlePost);
  post->setMethod(HTTP_POST);
  post->setMaxContentLength(kMaxJsonBodyBytes);
  server.addHandler(post);
}
