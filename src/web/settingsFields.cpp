#include "web/settingsFields.h"

#include "Commands.h"
#include "board/boardConfig.h"
#include "settings/Settings.h"
#include "settings/activityNames.h"
#include "settings/clockFaceNames.h"
#include "settings/splitClockNames.h"
#include "web/jsonFields.h"

namespace {

constexpr int32_t kAbsent = SettingsPatch::kAbsent;
constexpr const char* kAnimationsError =
  "animations must be an object with thinking, reading, editing, shell, delegating or mcp";

}

bool readAnimations(JsonVariantConst value, int8_t* out, const char*& error) {
  if (value.isNull()) {
    return true;
  }

  if (!value.is<JsonObjectConst>()) {
    error = kAnimationsError;
    return false;
  }

  for (JsonPairConst entry : value.as<JsonObjectConst>()) {
    Activity activity = Activity::THINKING;

    if (!parseActivity(entry.key().c_str(), activity)) {
      error = kAnimationsError;
      return false;
    }

    int8_t animation = kAbsent;

    if (!readWorkingAnimation(entry.value(), animation) || animation < 0) {
      error = kWorkingAnimationError;
      return false;
    }

    out[static_cast<size_t>(activity)] = animation;
  }

  return true;
}

bool readClockFace(JsonVariantConst value, int8_t& out, const char*& error) {
  if (value.isNull()) {
    return true;
  }

  if (!Board::kHasClockHalf) {
    error = "clockFace is only available on strich";
    return false;
  }

  ClockFace face = ClockFace::STAIRCASE;

  if (!value.is<const char*>() || !parseClockFace(value.as<const char*>(), face)) {
    error = "clockFace must be staircase or tall";
    return false;
  }

  out = static_cast<int8_t>(face);
  return true;
}

bool readSplitClock(JsonVariantConst value, int8_t& out, const char*& error) {
  if (value.isNull()) {
    return true;
  }

  if (!Board::kHasClockHalf) {
    error = "splitClock is only available on strich";
    return false;
  }

  SplitClock style = SplitClock::DIGITS;

  if (!value.is<const char*>() || !parseSplitClock(value.as<const char*>(), style)) {
    error = "splitClock must be digits, squares, stagger or bar";
    return false;
  }

  out = static_cast<int8_t>(style);
  return true;
}

bool readBatterySleep(JsonVariantConst value, int32_t& out, const char*& error) {
  if (value.isNull()) {
    return true;
  }

  if (!Board::kHasVbusSense) {
    error = "batterySleepMin is only available on strich";
    return false;
  }

  if (!readNumber(value, Settings::kMaxBatterySleepMin, out)) {
    error = "batterySleepMin must be a whole number between 0 and 240";
    return false;
  }

  return true;
}
