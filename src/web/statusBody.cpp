#include "web/statusBody.h"

#include "board/boardConfig.h"
#include "network/BootCounter.h"
#include "network/WifiSetup.h"
#include "power/Power.h"
#include "power/VbusSense.h"
#include "settings/Settings.h"
#include "settings/activityNames.h"
#include "state/SessionTable.h"
#include "state/State.h"
#include "state/stateNames.h"
#include "touch/Touch.h"
#include "touch/touchEventNames.h"
#include "version.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <cmath>

namespace {

constexpr uint32_t kMsPerSecond = 1000;

/**
 * Adds the context fill, its age and the stale flag.
 * @param {JsonDocument&} doc - status object being built
 * @param {const StateMachine&} state - state machine the context values come from
 * @param {uint32_t} nowMs - millis() at the time of the call
 * @returns {void}
 */
void addContext(JsonDocument& doc, const StateMachine& state, uint32_t nowMs) {
  doc["context"] = state.contextPercent();
  doc["contextStale"] = state.contextStale(nowMs);

  if (state.hasContext()) {
    doc["contextAgeS"] = state.contextAgeMs(nowMs) / kMsPerSecond;
  } else {
    doc["contextAgeS"] = nullptr;
  }
}

/**
 * Adds the usage share, its age and the stale flag.
 * @param {JsonDocument&} doc - status object being built
 * @param {const StateMachine&} state - state machine the usage values come from
 * @param {uint32_t} nowMs - millis() at the time of the call
 * @returns {void}
 */
void addUsage(JsonDocument& doc, const StateMachine& state, uint32_t nowMs) {
  doc["usage"] = state.usagePercent();
  doc["usageStale"] = state.usageStale(nowMs);

  if (state.hasUsage()) {
    doc["usageAgeS"] = state.usageAgeMs(nowMs) / kMsPerSecond;
  } else {
    doc["usageAgeS"] = nullptr;
  }
}

/**
 * Adds the snapshot of one touch electrode.
 * @param {JsonDocument&} doc - status object being built
 * @param {const char*} key - member the snapshot is written to
 * @param {const Touch&} touch - touch input the snapshot is copied from
 * @param {uint32_t} nowMs - millis() at the time of the call
 * @returns {void}
 */
void addTouch(JsonDocument& doc, const char* key, const Touch& touch, uint32_t nowMs) {
  const TouchStatus status = touch.status();
  JsonObject object = doc[key].to<JsonObject>();
  object["raw"] = status.raw;
  object["baseline"] = status.baseline;
  object["touched"] = status.touched;
  object["proximity"] = status.proximity;
  object["lastEvent"] = touchEventName(status.lastEvent);

  if (status.lastEvent == TouchEvent::NONE) {
    object["lastEventAgeS"] = nullptr;
  } else {
    object["lastEventAgeS"] = (nowMs - status.lastEventMs) / kMsPerSecond;
  }

  object["restarts"] = status.restarts;
}

/**
 * Adds the session table.
 * @param {JsonDocument&} doc - status object being built
 * @param {const StateMachine&} state - state machine the table is copied from
 * @returns {void}
 */
void addSessions(JsonDocument& doc, const StateMachine& state) {
  Session copies[SessionTable::kMaxSessions];
  const size_t count = state.sessions(copies, SessionTable::kMaxSessions);
  JsonArray list = doc["sessions"].to<JsonArray>();

  for (size_t i = 0; i < count; i++) {
    JsonObject session = list.add<JsonObject>();
    session["id"] = copies[i].id;
    session["state"] = stateName(copies[i].state);
    session["activity"] = activityName(static_cast<Activity>(copies[i].activity));
  }
}

}

String statusBody(const StateMachine& state, const Settings& settings, const Touch& touch,
                  const Touch& touchRight) {
  const uint32_t nowMs = millis();
  const float temperature = Power::temperatureC();

  JsonDocument doc;
  doc["state"] = stateName(state.current());
  doc["view"] = viewName(state.view());
  doc["animation"] = state.animationName();
  doc["uptimeS"] = nowMs / kMsPerSecond;
  doc["reset"] = BootCounter::resetReason();
  doc["name"] = settings.name();
  doc["board"] = Board::kDefaultName;
  doc["ip"] = WifiSetup::ipString();
  doc["rssi"] = WifiSetup::rssi();
  doc["version"] = kFirmwareVersion;
  doc["powerSave"] = settings.powerSave();
  doc["nightSleep"] = settings.nightSleep();
  doc["nightOff"] = settings.nightOff();
  doc["brightness"] = state.brightness();
  doc["dimmed"] = state.dimmed();
  doc["offReason"] = offReasonName(state.offReason());

  if (std::isnan(temperature)) {
    doc["temperatureC"] = nullptr;
  } else {
    doc["temperatureC"] = roundf(temperature * 10.0f) / 10.0f;
  }

  addContext(doc, state, nowMs);
  addUsage(doc, state, nowMs);
  addSessions(doc, state);
  addTouch(doc, "touch", touch, nowMs);

  if (Board::kHasClockHalf) {
    addTouch(doc, "touchRight", touchRight, nowMs);
  }

  if (Board::kHasVbusSense) {
    JsonObject power = doc["power"].to<JsonObject>();
    power["usb"] = VbusSense::usbPresent();
    power["vbusMv"] = VbusSense::vbusMv();
    const int percent = VbusSense::percent();

    if (percent >= 0) {
      power["percent"] = percent;
    }
  }

  String body;
  serializeJson(doc, body);
  return body;
}
