#include "web/percentRoutes.h"

#include "Commands.h"
#include "web/apiCommon.h"
#include "web/jsonFields.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>
#include <ESPAsyncWebServer.h>

#include <atomic>

namespace {

constexpr int32_t kMaxPercent = 100;
constexpr const char* kPercentError = "percent must be a whole number between 0 and 100";

/**
 * Hands a reported percentage to loop() through the given slot.
 * @param {AsyncWebServerRequest*} request - request to answer
 * @param {JsonVariant&} json - parsed request body
 * @param {std::atomic<int16_t>&} slot - command the value is written to
 * @returns {void}
 */
void handlePost(AsyncWebServerRequest* request, JsonVariant& json, std::atomic<int16_t>& slot) {
  JsonVariantConst value = json["percent"];
  int32_t percent = 0;

  // percent is the only required field of the API, readNumber accepts an absent one
  if (value.isNull() || !readNumber(value, kMaxPercent, percent)) {
    sendApiError(request, kPercentError);
    return;
  }

  slot = static_cast<int16_t>(percent);

  JsonDocument doc;
  doc["percent"] = percent;

  sendApiJson(request, doc);
}

/**
 * Registers one POST route whose body carries a single percent field.
 * @param {AsyncWebServer&} server - server the route is added to
 * @param {const char*} path - exact path to answer
 * @param {std::atomic<int16_t>&} slot - command the value is written to
 * @returns {void}
 */
void registerPercentRoute(AsyncWebServer& server, const char* path, std::atomic<int16_t>& slot) {
  auto* post = new AsyncCallbackJsonWebHandler(
      AsyncURIMatcher::exact(path),
      [&slot](AsyncWebServerRequest* request, JsonVariant& json) { handlePost(request, json, slot); });
  post->setMethod(HTTP_POST);
  post->setMaxContentLength(kMaxJsonBodyBytes);
  server.addHandler(post);
}

}

void registerPercentRoutes(AsyncWebServer& server, Commands& commands) {
  registerPercentRoute(server, "/context", commands.contextPercent);
  registerPercentRoute(server, "/usage", commands.usagePercent);
}
