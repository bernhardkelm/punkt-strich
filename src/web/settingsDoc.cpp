#include "web/settingsDoc.h"

#include "Commands.h"
#include "board/boardConfig.h"
#include "settings/Settings.h"
#include "settings/activityNames.h"
#include "settings/clockFaceNames.h"
#include "settings/clockMinutes.h"
#include "settings/splitClockNames.h"
#include "settings/workingAnimationNames.h"

namespace {

constexpr int32_t kAbsent = SettingsPatch::kAbsent;

/**
 * Adds the animation of every activity, a patched one overrides the cached value.
 * @param {JsonDocument&} doc - settings object being built
 * @param {const Settings&} settings - settings store the cached values are read from
 * @param {const SettingsPatch&} patch - validated fields, an empty patch reports the cached values
 * @returns {void}
 */
void addAnimations(JsonDocument& doc, const Settings& settings, const SettingsPatch& patch) {
  JsonObject animations = doc["animations"].to<JsonObject>();

  for (uint8_t i = 0; i < kActivityCount; i++) {
    const Activity activity = static_cast<Activity>(i);
    const WorkingAnimation animation =
      patch.animations[i] == kAbsent ? settings.activityAnimation(activity)
                                     : static_cast<WorkingAnimation>(patch.animations[i]);
    animations[activityName(activity)] = workingAnimationName(animation);
  }
}

}

JsonDocument settingsDoc(const Settings& settings, const SettingsPatch& patch) {
  const uint16_t sleepAt =
    patch.sleepAt == kAbsent ? settings.sleepAt() : static_cast<uint16_t>(patch.sleepAt);
  char sleepAtText[kClockTextSize] = {};
  formatClockMinutes(sleepAt, sleepAtText, sizeof(sleepAtText));
  const uint16_t wakeAt =
    patch.wakeAt == kAbsent ? settings.wakeAt() : static_cast<uint16_t>(patch.wakeAt);
  char wakeAtText[kClockTextSize] = {};
  formatClockMinutes(wakeAt, wakeAtText, sizeof(wakeAtText));

  JsonDocument doc;
  doc["brightness"] = patch.brightness == kAbsent ? settings.brightness() : patch.brightness;
  doc["autoDim"] = patch.autoDim == kAbsent ? settings.autoDim() : patch.autoDim == 1;
  doc["nightBrightness"] = patch.nightBrightness == kAbsent ? settings.nightBrightness()
                                                            : patch.nightBrightness;
  doc["city"] = patch.city[0] == '\0' ? settings.city() : patch.city;
  doc["dimTimeoutMin"] =
    patch.dimTimeoutMin == kAbsent ? settings.dimTimeoutMin() : patch.dimTimeoutMin;
  doc["offTimeoutMin"] =
    patch.offTimeoutMin == kAbsent ? settings.offTimeoutMin() : patch.offTimeoutMin;

  if (Board::kHasVbusSense) {
    doc["batterySleepMin"] = patch.batterySleepMin == kAbsent ? settings.batterySleepMin()
                                                              : patch.batterySleepMin;
  }

  doc["clockTimeoutMin"] =
    patch.clockTimeoutMin == kAbsent ? settings.clockTimeoutMin() : patch.clockTimeoutMin;
  doc["nightSleep"] =
    patch.nightSleep == kAbsent ? settings.nightSleep() : patch.nightSleep == 1;
  doc["sleepAt"] = sleepAtText;
  doc["nightOff"] = patch.nightOff == kAbsent ? settings.nightOff() : patch.nightOff == 1;

  if (Board::kHasClockHalf) {
    const ClockFace face = patch.clockFace == kAbsent
                             ? settings.clockFace()
                             : static_cast<ClockFace>(patch.clockFace);
    doc["clockFace"] = clockFaceName(face);
    const SplitClock style = patch.splitClock == kAbsent
                               ? settings.splitClock()
                               : static_cast<SplitClock>(patch.splitClock);
    doc["splitClock"] = splitClockName(style);
  }

  doc["wakeAt"] = wakeAtText;
  doc["powerSave"] = patch.powerSave == kAbsent ? settings.powerSave() : patch.powerSave == 1;
  addAnimations(doc, settings, patch);
  doc["touchTrigger"] =
    patch.touchTrigger == kAbsent ? settings.touchTrigger() : patch.touchTrigger;
  doc["touchHover"] =
    patch.touchHover == kAbsent ? settings.touchHover() : patch.touchHover;
  doc["debugTouch"] =
    patch.debugTouch == kAbsent ? settings.debugTouch() : patch.debugTouch == 1;
  doc["name"] = patch.name[0] == '\0' ? settings.name() : patch.name;
  return doc;
}
