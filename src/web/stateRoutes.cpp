#include "web/stateRoutes.h"

#include "Commands.h"
#include "settings/Settings.h"
#include "settings/activityNames.h"
#include "state/State.h"
#include "state/stateNames.h"
#include "web/apiCommon.h"
#include "web/jsonFields.h"
#include "web/statusBody.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>
#include <ESPAsyncWebServer.h>

namespace {

constexpr const char* kSessionError = "session must be at most 32 characters";
constexpr const char* kTimestampError = "ts must be a whole number of milliseconds";
constexpr const char* kActivityError =
  "activity must be thinking, reading, editing, shell, delegating or mcp";

Commands* commandQueue = nullptr;
Settings* settingsStore = nullptr;
const StateMachine* stateMachine = nullptr;
const Touch* touchInput = nullptr;
const Touch* touchRightInput = nullptr;

/**
 * Reads the optional session id of a request.
 * @param {JsonVariantConst} value - field value, null when the key is absent
 * @param {const char*&} out - receives the id, the empty string when the key is absent
 * @returns {bool} - false when the key is present but not a string of at most 32 characters
 */
bool readSession(JsonVariantConst value, const char*& out) {
  if (!readText(value, kSessionIdMax, out)) {
    return false;
  }

  if (out == nullptr) {
    out = "";
  }

  return true;
}

/**
 * Reads the optional activity a working request reports, which the settings map to an animation.
 * @param {JsonVariantConst} value - field value, null when the key is absent
 * @param {uint8_t&} out - receives the Activity value, kDefaultActivity when the key is absent
 * @returns {bool} - false when the key is present but not a known activity name
 */
bool readActivity(JsonVariantConst value, uint8_t& out) {
  out = kDefaultActivity;

  if (value.isNull()) {
    return true;
  }

  const char* name = value.is<const char*>() ? value.as<const char*>() : nullptr;
  Activity activity = Activity::THINKING;

  if (!parseActivity(name, activity)) {
    return false;
  }

  out = static_cast<uint8_t>(activity);
  return true;
}

/**
 * Reads the optional sender timestamp the board orders the requests of one session by.
 * @param {JsonVariantConst} value - field value, null when the key is absent
 * @param {uint64_t&} out - receives the timestamp in ms, 0 when the key is absent
 * @returns {bool} - false when the key is present but not a whole non-negative number
 */
bool readTimestamp(JsonVariantConst value, uint64_t& out) {
  out = 0;

  if (value.isNull()) {
    return true;
  }

  if (!value.is<uint64_t>()) {
    return false;
  }

  out = value.as<uint64_t>();
  return true;
}

/**
 * Answers GET /state with the status object.
 * @param {AsyncWebServerRequest*} request - request to answer
 * @returns {void}
 */
void handleGet(AsyncWebServerRequest* request) {
  logApi("GET", "/state", stateName(stateMachine->current()));
  request->send(200, kJsonContentType,
                statusBody(*stateMachine, *settingsStore, *touchInput, *touchRightInput));
}

/**
 * Enqueues the requested state for loop() to apply.
 * @param {AsyncWebServerRequest*} request - request to answer
 * @param {JsonVariant&} json - parsed request body
 * @returns {void}
 */
void handlePost(AsyncWebServerRequest* request, JsonVariant& json) {
  JsonVariantConst value = json["state"];
  const char* name = value.is<const char*>() ? value.as<const char*>() : nullptr;
  PunktState requested = PunktState::IDLE;
  const char* session = "";
  const bool namedActivity = !json["activity"].isNull();
  uint8_t activity = kDefaultActivity;
  int8_t animation = -1;
  uint64_t ts = 0;

  if (!parseState(name, requested)) {
    sendApiError(request, "unknown state");
    return;
  }

  if (!readSession(json["session"], session)) {
    sendApiError(request, kSessionError);
    return;
  }

  if (!readActivity(json["activity"], activity)) {
    sendApiError(request, kActivityError);
    return;
  }

  if (!readWorkingAnimation(json["animation"], animation)) {
    sendApiError(request, kWorkingAnimationError);
    return;
  }

  if (!readTimestamp(json["ts"], ts)) {
    sendApiError(request, kTimestampError);
    return;
  }

  if (!commandQueue->postState(session, requested, activity, animation, ts)) {
    sendApiError(request, kQueueFullError, 503);
    return;
  }

  JsonDocument doc;
  doc["state"] = stateName(requested);

  if (session[0] != '\0') {
    doc["session"] = session;
  }

  if (namedActivity) {
    doc["activity"] = activityName(static_cast<Activity>(activity));
  }

  sendApiJson(request, doc, stateName(requested));
}

}

void registerStateRoutes(AsyncWebServer& server, Commands& commands, Settings& settings,
                         const StateMachine& state, const Touch& touch, const Touch& touchRight) {
  commandQueue = &commands;
  settingsStore = &settings;
  stateMachine = &state;
  touchInput = &touch;
  touchRightInput = &touchRight;

  server.on(AsyncURIMatcher::exact("/state"), HTTP_GET, handleGet);

  auto* post = new AsyncCallbackJsonWebHandler(AsyncURIMatcher::exact("/state"), handlePost);
  post->setMethod(HTTP_POST);
  post->setMaxContentLength(kMaxJsonBodyBytes);
  server.addHandler(post);
}
