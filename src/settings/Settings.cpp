#include "settings/Settings.h"

#include "settings/cities.h"

#include <cstdio>
#include <cstring>

namespace {

/**
 * Returns the city a name stands for, the default city for a null or unknown name.
 * @param {const char*} name - candidate IANA name, may be null
 * @returns {const City&} - table entry
 */
const City& cityOrDefault(const char* name) {
  const City* city = findCity(name);
  return city == nullptr ? defaultCity() : *city;
}

/**
 * Limits a value to an inclusive upper bound.
 * @param {uint16_t} value - raw value
 * @param {uint16_t} maximum - highest accepted value
 * @returns {uint16_t} - value or maximum
 */
uint16_t clamped(uint16_t value, uint16_t maximum) {
  return value > maximum ? maximum : value;
}

}

void Settings::begin() {
  prefs_.begin(kNamespace, false);

  // going through the setters persists the defaults for missing keys, NVS skips identical writes
  setBrightness(prefs_.getUChar(kBrightnessKey, kDefaultBrightness));
  setDimTimeoutMin(prefs_.getUShort(kDimTimeoutKey, kDefaultDimTimeoutMin));
  setOffTimeoutMin(prefs_.getUShort(kOffTimeoutKey, kDefaultOffTimeoutMin));
  setBatterySleepMin(prefs_.getUShort(kBatterySleepKey, kDefaultBatterySleepMin));
  setClockTimeoutMin(prefs_.getUShort(kClockTimeoutKey, kDefaultClockTimeoutMin));
  loadActivityAnimations();
  setPowerSave(prefs_.getBool(kPowerSaveKey, kDefaultPowerSave));
  setDebugTouch(prefs_.getBool(kDebugTouchKey, kDefaultDebugTouch));
  setNightSleep(prefs_.getBool(kNightSleepKey, kDefaultNightSleep));
  setSleepAt(prefs_.getUShort(kSleepAtKey, kDefaultSleepAt));
  setNightOff(prefs_.getBool(kNightOffKey, kDefaultNightOff));
  setClockFace(static_cast<ClockFace>(
    prefs_.getUChar(kClockFaceKey, static_cast<uint8_t>(kDefaultClockFace))));
  setSplitClock(static_cast<SplitClock>(
    prefs_.getUChar(kSplitClockKey, static_cast<uint8_t>(kDefaultSplitClock))));
  setWakeAt(prefs_.getUShort(kWakeAtKey, kDefaultWakeAt));
  setAutoDim(prefs_.getBool(kAutoDimKey, kDefaultAutoDim));
  setNightBrightness(prefs_.getUChar(kNightBrightnessKey, kDefaultNightBrightness));
  setTouchTrigger(prefs_.getUChar(kTouchTriggerKey, kDefaultTouchTrigger));
  setTouchHover(prefs_.getUChar(kTouchHoverKey, kDefaultTouchHover));

  char city[kCityNameMax + 1] = {};
  prefs_.getString(kCityKey, city, sizeof(city));
  setCity(city);

  char name[kNameMax + 1] = {};
  prefs_.getString(kNameKey, name, sizeof(name));
  setName(name);
}

void Settings::reset() {
  prefs_.clear();
}

uint8_t Settings::brightness() const {
  return brightness_;
}

void Settings::setBrightness(uint8_t level) {
  brightness_ = static_cast<uint8_t>(clamped(level, kMaxBrightness));
  prefs_.putUChar(kBrightnessKey, brightness_);
}

uint16_t Settings::dimTimeoutMin() const {
  return dimTimeoutMin_;
}

void Settings::setDimTimeoutMin(uint16_t minutes) {
  dimTimeoutMin_ = clamped(minutes, kMaxTimeoutMin);
  prefs_.putUShort(kDimTimeoutKey, dimTimeoutMin_);
}

uint16_t Settings::offTimeoutMin() const {
  return offTimeoutMin_;
}

void Settings::setOffTimeoutMin(uint16_t minutes) {
  offTimeoutMin_ = clamped(minutes, kMaxTimeoutMin);
  prefs_.putUShort(kOffTimeoutKey, offTimeoutMin_);
}

uint16_t Settings::batterySleepMin() const {
  return batterySleepMin_;
}

void Settings::setBatterySleepMin(uint16_t minutes) {
  batterySleepMin_ = clamped(minutes, kMaxBatterySleepMin);
  prefs_.putUShort(kBatterySleepKey, batterySleepMin_);
}

uint16_t Settings::clockTimeoutMin() const {
  return clockTimeoutMin_;
}

void Settings::setClockTimeoutMin(uint16_t minutes) {
  clockTimeoutMin_ = clamped(minutes, kMaxTimeoutMin);
  prefs_.putUShort(kClockTimeoutKey, clockTimeoutMin_);
}

WorkingAnimation Settings::activityAnimation(Activity activity) const {
  const size_t index = static_cast<size_t>(activity);

  if (index >= kActivityCount) {
    return kDefaultActivityAnimations[kDefaultActivity];
  }

  return activityAnimation_[index];
}

void Settings::setActivityAnimation(Activity activity, WorkingAnimation animation) {
  if (!cacheActivityAnimation(activity, animation)) {
    return;
  }

  char key[kAnimationKeySize] = {};
  animationKey(activity, key);
  prefs_.putUChar(key, static_cast<uint8_t>(activityAnimation_[static_cast<size_t>(activity)]));
}

bool Settings::cacheActivityAnimation(Activity activity, WorkingAnimation animation) {
  const size_t index = static_cast<size_t>(activity);

  if (index >= kActivityCount) {
    return false;
  }

  const bool known = static_cast<uint8_t>(animation) < kWorkingAnimationCount;
  activityAnimation_[index] = known ? animation : kDefaultActivityAnimations[index];
  return true;
}

void Settings::animationKey(Activity activity, char* out) {
  snprintf(out, kAnimationKeySize, "anim%u", static_cast<unsigned>(activity));
}

void Settings::loadActivityAnimations() {
  for (uint8_t i = 0; i < kActivityCount; i++) {
    const Activity activity = static_cast<Activity>(i);
    const uint8_t fallback = static_cast<uint8_t>(kDefaultActivityAnimations[i]);
    char key[kAnimationKeySize] = {};
    animationKey(activity, key);
    cacheActivityAnimation(activity, static_cast<WorkingAnimation>(prefs_.getUChar(key, fallback)));
  }
}

bool Settings::powerSave() const {
  return powerSave_;
}

void Settings::setPowerSave(bool enabled) {
  powerSave_ = enabled;
  prefs_.putBool(kPowerSaveKey, powerSave_);
}

bool Settings::debugTouch() const {
  return debugTouch_;
}

void Settings::setDebugTouch(bool enabled) {
  debugTouch_ = enabled;
  prefs_.putBool(kDebugTouchKey, debugTouch_);
}

bool Settings::nightSleep() const {
  return nightSleep_;
}

void Settings::setNightSleep(bool enabled) {
  nightSleep_ = enabled;
  prefs_.putBool(kNightSleepKey, nightSleep_);
}

uint16_t Settings::sleepAt() const {
  return sleepAt_;
}

void Settings::setSleepAt(uint16_t minutes) {
  sleepAt_ = clamped(minutes, kMaxClockMin);
  prefs_.putUShort(kSleepAtKey, sleepAt_);
}

bool Settings::nightOff() const {
  return nightOff_;
}

void Settings::setNightOff(bool enabled) {
  nightOff_ = enabled;
  prefs_.putBool(kNightOffKey, nightOff_);
}

ClockFace Settings::clockFace() const {
  return clockFace_;
}

void Settings::setClockFace(ClockFace face) {
  const bool known = static_cast<uint8_t>(face) < kClockFaceCount;
  clockFace_ = known ? face : kDefaultClockFace;
  prefs_.putUChar(kClockFaceKey, static_cast<uint8_t>(clockFace_));
}

SplitClock Settings::splitClock() const {
  return splitClock_;
}

void Settings::setSplitClock(SplitClock style) {
  const bool known = static_cast<uint8_t>(style) < kSplitClockCount;
  splitClock_ = known ? style : kDefaultSplitClock;
  prefs_.putUChar(kSplitClockKey, static_cast<uint8_t>(splitClock_));
}

uint16_t Settings::wakeAt() const {
  return wakeAt_;
}

void Settings::setWakeAt(uint16_t minutes) {
  wakeAt_ = clamped(minutes, kMaxClockMin);
  prefs_.putUShort(kWakeAtKey, wakeAt_);
}

bool Settings::autoDim() const {
  return autoDim_;
}

void Settings::setAutoDim(bool enabled) {
  autoDim_ = enabled;
  prefs_.putBool(kAutoDimKey, autoDim_);
}

uint8_t Settings::nightBrightness() const {
  return nightBrightness_;
}

void Settings::setNightBrightness(uint8_t level) {
  nightBrightness_ = static_cast<uint8_t>(clamped(level, kMaxBrightness));
  prefs_.putUChar(kNightBrightnessKey, nightBrightness_);
}

float Settings::latitude() const {
  return cityOrDefault(city_).latitude;
}

float Settings::longitude() const {
  return cityOrDefault(city_).longitude;
}

uint8_t Settings::touchTrigger() const {
  return touchTrigger_;
}

void Settings::setTouchTrigger(uint8_t percent) {
  const uint8_t floored = percent < kMinTouchTrigger ? kMinTouchTrigger : percent;
  touchTrigger_ = static_cast<uint8_t>(clamped(floored, kMaxTouchTrigger));
  prefs_.putUChar(kTouchTriggerKey, touchTrigger_);
}

uint8_t Settings::touchHover() const {
  return touchHover_;
}

void Settings::setTouchHover(uint8_t percent) {
  touchHover_ = static_cast<uint8_t>(clamped(percent, kMaxTouchHover));
  prefs_.putUChar(kTouchHoverKey, touchHover_);
}

const char* Settings::city() const {
  return city_;
}

void Settings::setCity(const char* name) {
  strncpy(city_, cityOrDefault(name).name, kCityNameMax);
  city_[kCityNameMax] = '\0';
  prefs_.putString(kCityKey, city_);
}

const char* Settings::timezone() const {
  return cityOrDefault(city_).posix;
}

const char* Settings::name() const {
  return name_;
}

void Settings::setName(const char* value) {
  const char* source = value == nullptr || value[0] == '\0' ? kDefaultName : value;
  strncpy(name_, source, kNameMax);
  name_[kNameMax] = '\0';
  prefs_.putString(kNameKey, name_);
}
