#include "app/patchSettings.h"

#include "Commands.h"
#include "settings/Settings.h"

void patchSettings(Settings& settings, const SettingsPatch& patch) {
  if (patch.brightness != SettingsPatch::kAbsent) {
    settings.setBrightness(static_cast<uint8_t>(patch.brightness));
  }

  if (patch.dimTimeoutMin != SettingsPatch::kAbsent) {
    settings.setDimTimeoutMin(static_cast<uint16_t>(patch.dimTimeoutMin));
  }

  if (patch.offTimeoutMin != SettingsPatch::kAbsent) {
    settings.setOffTimeoutMin(static_cast<uint16_t>(patch.offTimeoutMin));
  }

  if (patch.batterySleepMin != SettingsPatch::kAbsent) {
    settings.setBatterySleepMin(static_cast<uint16_t>(patch.batterySleepMin));
  }

  if (patch.clockTimeoutMin != SettingsPatch::kAbsent) {
    settings.setClockTimeoutMin(static_cast<uint16_t>(patch.clockTimeoutMin));
  }

  for (uint8_t i = 0; i < kActivityCount; i++) {
    if (patch.animations[i] == SettingsPatch::kAbsent) {
      continue;
    }

    settings.setActivityAnimation(static_cast<Activity>(i),
                                  static_cast<WorkingAnimation>(patch.animations[i]));
  }

  if (patch.touchTrigger != SettingsPatch::kAbsent) {
    settings.setTouchTrigger(static_cast<uint8_t>(patch.touchTrigger));
  }

  if (patch.touchHover != SettingsPatch::kAbsent) {
    settings.setTouchHover(static_cast<uint8_t>(patch.touchHover));
  }

  if (patch.city[0] != '\0') {
    settings.setCity(patch.city);
  }

  // the network side read the name at boot, a new one reaches the hostname on the next restart
  if (patch.name[0] != '\0') {
    settings.setName(patch.name);
  }

  if (patch.powerSave != SettingsPatch::kAbsent) {
    settings.setPowerSave(patch.powerSave == 1);
  }

  if (patch.debugTouch != SettingsPatch::kAbsent) {
    settings.setDebugTouch(patch.debugTouch == 1);
  }

  if (patch.nightSleep != SettingsPatch::kAbsent) {
    settings.setNightSleep(patch.nightSleep == 1);
  }

  if (patch.sleepAt != SettingsPatch::kAbsent) {
    settings.setSleepAt(static_cast<uint16_t>(patch.sleepAt));
  }

  if (patch.nightOff != SettingsPatch::kAbsent) {
    settings.setNightOff(patch.nightOff == 1);
  }

  if (patch.clockFace != SettingsPatch::kAbsent) {
    settings.setClockFace(static_cast<ClockFace>(patch.clockFace));
  }

  if (patch.splitClock != SettingsPatch::kAbsent) {
    settings.setSplitClock(static_cast<SplitClock>(patch.splitClock));
  }

  if (patch.wakeAt != SettingsPatch::kAbsent) {
    settings.setWakeAt(static_cast<uint16_t>(patch.wakeAt));
  }

  if (patch.autoDim != SettingsPatch::kAbsent) {
    settings.setAutoDim(patch.autoDim == 1);
  }

  if (patch.nightBrightness != SettingsPatch::kAbsent) {
    settings.setNightBrightness(static_cast<uint8_t>(patch.nightBrightness));
  }
}
