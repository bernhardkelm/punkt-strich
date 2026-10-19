#include "web/previewRoutes.h"

#include "Commands.h"
#include "state/State.h"
#include "web/apiCommon.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>
#include <ESPAsyncWebServer.h>

namespace {

constexpr const char* kAnimationError = "animation must be one of the names GET /preview lists";

Commands* commandQueue = nullptr;
const StateMachine* stateMachine = nullptr;

/**
 * Answers GET /preview with every animation name the state machine owns, in machine order.
 * @param {AsyncWebServerRequest*} request - request to answer
 * @returns {void}
 */
void handleGet(AsyncWebServerRequest* request) {
  JsonDocument doc;
  JsonArray animations = doc["animations"].to<JsonArray>();

  for (size_t i = 0; i < StateMachine::kAnimationCount; i++) {
    animations.add(stateMachine->animationNameAt(i));
  }

  sendApiJson(request, doc, "");
}

/**
 * Hands the requested animation index to loop() through the preview command, and answers 400
 * when the name is not one GET /preview lists.
 * @param {AsyncWebServerRequest*} request - request to answer
 * @param {JsonVariant&} json - parsed request body
 * @returns {void}
 */
void handlePost(AsyncWebServerRequest* request, JsonVariant& json) {
  JsonVariantConst value = json["animation"];
  const char* name = value.is<const char*>() ? value.as<const char*>() : nullptr;
  const int index = stateMachine->animationIndex(name);

  if (index < 0) {
    sendApiError(request, kAnimationError);
    return;
  }

  commandQueue->previewRequest = static_cast<int8_t>(index);

  JsonDocument doc;
  doc["animation"] = name;

  sendApiJson(request, doc);
}

}

void registerPreviewRoutes(AsyncWebServer& server, Commands& commands, const StateMachine& state) {
  commandQueue = &commands;
  stateMachine = &state;

  server.on(AsyncURIMatcher::exact("/preview"), HTTP_GET, handleGet);

  auto* post = new AsyncCallbackJsonWebHandler(AsyncURIMatcher::exact("/preview"), handlePost);
  post->setMethod(HTTP_POST);
  post->setMaxContentLength(kMaxJsonBodyBytes);
  server.addHandler(post);
}
