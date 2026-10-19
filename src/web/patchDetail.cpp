#include "web/patchDetail.h"

#include "Commands.h"
#include "settings/activityNames.h"
#include "settings/clockFaceNames.h"
#include "settings/clockMinutes.h"
#include "settings/splitClockNames.h"
#include "settings/workingAnimationNames.h"

namespace {

constexpr int32_t kAbsent = SettingsPatch::kAbsent;

}

String patchDetail(const SettingsPatch& patch) {
  String detail;

  if (patch.name[0] != '\0') {
    detail += " name=";
    detail += patch.name;
  }

  if (patch.brightness != kAbsent) {
    detail += " brightness=";
    detail += patch.brightness;
  }

  if (patch.dimTimeoutMin != kAbsent) {
    detail += " dimTimeoutMin=";
    detail += patch.dimTimeoutMin;
  }

  if (patch.offTimeoutMin != kAbsent) {
    detail += " offTimeoutMin=";
    detail += patch.offTimeoutMin;
  }

  if (patch.clockTimeoutMin != kAbsent) {
    detail += " clockTimeoutMin=";
    detail += patch.clockTimeoutMin;
  }

  for (uint8_t i = 0; i < kActivityCount; i++) {
    if (patch.animations[i] == kAbsent) {
      continue;
    }

    detail += " animations.";
    detail += activityName(static_cast<Activity>(i));
    detail += "=";
    detail += workingAnimationName(static_cast<WorkingAnimation>(patch.animations[i]));
  }

  if (patch.touchTrigger != kAbsent) {
    detail += " touchTrigger=";
    detail += patch.touchTrigger;
  }

  if (patch.touchHover != kAbsent) {
    detail += " touchHover=";
    detail += patch.touchHover;
  }

  if (patch.city[0] != '\0') {
    detail += " city=";
    detail += patch.city;
  }

  if (patch.powerSave != kAbsent) {
    detail += " powerSave=";
    detail += patch.powerSave;
  }

  if (patch.debugTouch != kAbsent) {
    detail += " debugTouch=";
    detail += patch.debugTouch;
  }

  if (patch.nightSleep != kAbsent) {
    detail += " nightSleep=";
    detail += patch.nightSleep;
  }

  if (patch.sleepAt != kAbsent) {
    char text[kClockTextSize] = {};
    formatClockMinutes(static_cast<uint16_t>(patch.sleepAt), text, sizeof(text));
    detail += " sleepAt=";
    detail += text;
  }

  if (patch.nightOff != kAbsent) {
    detail += " nightOff=";
    detail += patch.nightOff;
  }

  if (patch.clockFace != kAbsent) {
    detail += " clockFace=";
    detail += clockFaceName(static_cast<ClockFace>(patch.clockFace));
  }

  if (patch.splitClock != kAbsent) {
    detail += " splitClock=";
    detail += splitClockName(static_cast<SplitClock>(patch.splitClock));
  }

  if (patch.batterySleepMin != kAbsent) {
    detail += " batterySleepMin=";
    detail += patch.batterySleepMin;
  }

  if (patch.wakeAt != kAbsent) {
    char text[kClockTextSize] = {};
    formatClockMinutes(static_cast<uint16_t>(patch.wakeAt), text, sizeof(text));
    detail += " wakeAt=";
    detail += text;
  }

  if (patch.autoDim != kAbsent) {
    detail += " autoDim=";
    detail += patch.autoDim;
  }

  if (patch.nightBrightness != kAbsent) {
    detail += " nightBrightness=";
    detail += patch.nightBrightness;
  }

  detail.trim();
  return detail;
}
