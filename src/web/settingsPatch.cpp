#include "web/settingsPatch.h"

#include "Commands.h"
#include "settings/Settings.h"
#include "web/jsonFields.h"
#include "web/settingsFields.h"

#include <cstring>

namespace {

constexpr int32_t kAbsent = SettingsPatch::kAbsent;

}

bool readSettingsPatch(JsonVariant& json, SettingsPatch& patch, const char*& error) {
  int32_t brightness = kAbsent;
  int32_t dimTimeoutMin = kAbsent;
  int32_t offTimeoutMin = kAbsent;
  int32_t clockTimeoutMin = kAbsent;
  int32_t batterySleepMin = kAbsent;
  int32_t touchTrigger = kAbsent;
  int32_t touchHover = kAbsent;
  int32_t sleepAt = kAbsent;
  int32_t wakeAt = kAbsent;
  int32_t nightBrightness = kAbsent;
  int8_t powerSave = kAbsent;
  int8_t debugTouch = kAbsent;
  int8_t nightSleep = kAbsent;
  int8_t nightOff = kAbsent;
  int8_t clockFace = kAbsent;
  int8_t splitClock = kAbsent;
  int8_t autoDim = kAbsent;
  const char* city = nullptr;
  const char* name = nullptr;

  if (!json.is<JsonObject>()) {
    error = "body must be a JSON object";
    return false;
  }

  if (!readName(json["name"], name)) {
    error = kNameError;
    return false;
  }

  if (!readNumber(json["brightness"], Settings::kMaxBrightness, brightness)) {
    error = "brightness must be a whole number between 0 and 15";
    return false;
  }

  if (!readNumber(json["dimTimeoutMin"], Settings::kMaxTimeoutMin, dimTimeoutMin) ||
      !readNumber(json["offTimeoutMin"], Settings::kMaxTimeoutMin, offTimeoutMin) ||
      !readNumber(json["clockTimeoutMin"], Settings::kMaxTimeoutMin, clockTimeoutMin)) {
    error = "timeouts must be whole numbers between 0 and 1440";
    return false;
  }

  if (!readFlag(json["powerSave"], powerSave) || !readFlag(json["debugTouch"], debugTouch)) {
    error = "powerSave and debugTouch must be true or false";
    return false;
  }

  if (!readNumber(json["touchTrigger"], Settings::kMaxTouchTrigger, touchTrigger) ||
      (touchTrigger != kAbsent && touchTrigger < Settings::kMinTouchTrigger)) {
    error = "touchTrigger must be a whole number between 5 and 250";
    return false;
  }

  if (!readNumber(json["touchHover"], Settings::kMaxTouchHover, touchHover)) {
    error = "touchHover must be a whole number between 0 and 50";
    return false;
  }

  if (!readCity(json["city"], city)) {
    error = kCityError;
    return false;
  }

  if (!readFlag(json["nightSleep"], nightSleep)) {
    error = "nightSleep must be true or false";
    return false;
  }

  if (!readClock(json["sleepAt"], sleepAt)) {
    error = "sleepAt must be a time like 22:00";
    return false;
  }

  if (!readFlag(json["nightOff"], nightOff)) {
    error = "nightOff must be true or false";
    return false;
  }

  if (!readClockFace(json["clockFace"], clockFace, error)) {
    return false;
  }

  if (!readSplitClock(json["splitClock"], splitClock, error)) {
    return false;
  }

  if (!readBatterySleep(json["batterySleepMin"], batterySleepMin, error)) {
    return false;
  }

  if (!readClock(json["wakeAt"], wakeAt)) {
    error = "wakeAt must be a time like 07:00";
    return false;
  }

  if (!readFlag(json["autoDim"], autoDim)) {
    error = "autoDim must be true or false";
    return false;
  }

  if (!readNumber(json["nightBrightness"], Settings::kMaxBrightness, nightBrightness)) {
    error = "nightBrightness must be a whole number between 0 and 15";
    return false;
  }

  if (!readAnimations(json["animations"], patch.animations, error)) {
    return false;
  }

  patch.brightness = static_cast<int16_t>(brightness);
  patch.dimTimeoutMin = static_cast<int16_t>(dimTimeoutMin);
  patch.offTimeoutMin = static_cast<int16_t>(offTimeoutMin);
  patch.clockTimeoutMin = static_cast<int16_t>(clockTimeoutMin);
  patch.touchTrigger = static_cast<int16_t>(touchTrigger);
  patch.touchHover = static_cast<int16_t>(touchHover);
  patch.powerSave = powerSave;
  patch.debugTouch = debugTouch;
  patch.nightSleep = nightSleep;
  patch.sleepAt = static_cast<int16_t>(sleepAt);
  patch.nightOff = nightOff;
  patch.clockFace = clockFace;
  patch.splitClock = splitClock;
  patch.batterySleepMin = static_cast<int16_t>(batterySleepMin);
  patch.wakeAt = static_cast<int16_t>(wakeAt);
  patch.autoDim = autoDim;
  patch.nightBrightness = static_cast<int16_t>(nightBrightness);

  if (name != nullptr) {
    strncpy(patch.name, name, Settings::kNameMax);
  }

  if (city != nullptr) {
    strncpy(patch.city, city, Settings::kCityNameMax);
  }

  return true;
}
