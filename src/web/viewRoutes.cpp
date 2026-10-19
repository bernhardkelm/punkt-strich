#include "web/viewRoutes.h"

#include "Commands.h"
#include "state/State.h"
#include "state/stateNames.h"
#include "web/apiCommon.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>
#include <ESPAsyncWebServer.h>
#include <cstring>

namespace {

constexpr const char* kViewError =
    StateMachine::kHasBatteryView ? "view must be clock, normal, usage, battery, off or toggle"
                                  : "view must be clock, normal, usage, off or toggle";
constexpr const char* kNoSessionError = "no session";
constexpr uint16_t kConflictStatus = 409;

Commands* commandQueue = nullptr;
const StateMachine* stateMachine = nullptr;

/**
 * Returns whether the board offers a view, BATTERY only exists with the battery view.
 * @param {View} view - parsed view
 * @returns {bool} - false for BATTERY on a board without StateMachine::kHasBatteryView
 */
bool viewOffered(View view) {
  return view != View::BATTERY || StateMachine::kHasBatteryView;
}

/**
 * Hands the requested view to loop(), the same switch the touch electrode triggers, and answers
 * 409 when the board would refuse the change. BATTERY is rejected like an unknown name where the
 * board does not offer it.
 * @param {AsyncWebServerRequest*} request - request to answer
 * @param {JsonVariant&} json - parsed request body
 * @returns {void}
 */
void handlePost(AsyncWebServerRequest* request, JsonVariant& json) {
  JsonVariantConst value = json["view"];
  const char* name = value.is<const char*>() ? value.as<const char*>() : nullptr;
  View requested = View::CLOCK;
  const bool toggle = name != nullptr && strcmp(name, "toggle") == 0;

  if (!toggle && (!parseView(name, requested) || !viewOffered(requested))) {
    sendApiError(request, kViewError);
    return;
  }

  if (!stateMachine->viewAllowed(toggle ? stateMachine->nextView() : requested)) {
    sendApiError(request, kNoSessionError, kConflictStatus);
    return;
  }

  commandQueue->viewRequest = toggle ? kViewToggle : static_cast<int8_t>(requested);

  JsonDocument doc;
  doc["view"] = name;

  sendApiJson(request, doc);
}

}

void registerViewRoutes(AsyncWebServer& server, Commands& commands, const StateMachine& state) {
  commandQueue = &commands;
  stateMachine = &state;

  auto* post = new AsyncCallbackJsonWebHandler(AsyncURIMatcher::exact("/view"), handlePost);
  post->setMethod(HTTP_POST);
  post->setMaxContentLength(kMaxJsonBodyBytes);
  server.addHandler(post);
}
