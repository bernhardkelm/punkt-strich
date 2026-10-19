#include "dimming/AutoDim.h"

#include "dimming/solarElevation.h"
#include "network/TimeSync.h"
#include "settings/Settings.h"

#include <Arduino.h>
#include <ctime>

namespace {

constexpr uint8_t kMaxLevel = 15;

/**
 * Maps an elevation onto the share of the way from the night level to the day level.
 * @param {float} elevationDeg - sun's elevation in degrees
 * @returns {float} - 0 at the night threshold, 1 at the day threshold, smoothstep between
 */
float dayShare(float elevationDeg) {
  const float span = AutoDim::kDayElevationDeg - AutoDim::kNightElevationDeg;
  const float raw = (elevationDeg - AutoDim::kNightElevationDeg) / span;
  const float t = raw < 0.0f ? 0.0f : (raw > 1.0f ? 1.0f : raw);
  return t * t * (3.0f - 2.0f * t);
}

/**
 * Interpolates between the night and the day level, rounded to the nearest level.
 * @param {uint8_t} night - level at and below the night threshold
 * @param {uint8_t} day - level at and above the day threshold
 * @param {float} share - 0 for the night level, 1 for the day level
 * @returns {uint8_t} - level, 0 to 15
 */
uint8_t levelBetween(uint8_t night, uint8_t day, float share) {
  const float exact = static_cast<float>(night) +
                      (static_cast<float>(day) - static_cast<float>(night)) * share;
  const float rounded = exact + 0.5f;
  const float bounded = rounded < 0.0f ? 0.0f : (rounded > kMaxLevel ? kMaxLevel : rounded);
  return static_cast<uint8_t>(bounded);
}

}

uint8_t AutoDim::refresh(uint32_t nowMs, const Settings& settings) {
  checkedMs_ = nowMs;
  checked_ = true;
  level_ = evaluate(settings);
  return level_;
}

bool AutoDim::tick(uint32_t nowMs, const Settings& settings, uint8_t& level) {
  if (checked_ && nowMs - checkedMs_ < kCheckIntervalMs) {
    return false;
  }

  checkedMs_ = nowMs;
  checked_ = true;
  const uint8_t next = evaluate(settings);

  if (next == level_) {
    return false;
  }

  level_ = next;
  level = next;
  return true;
}

uint8_t AutoDim::level() const {
  return level_;
}

uint8_t AutoDim::evaluate(const Settings& settings) {
  if (!settings.autoDim() || !TimeSync::synced()) {
    return settings.brightness();
  }

  const float elevation = solarElevationDeg(time(nullptr), settings.latitude(), settings.longitude());
  const uint8_t next = levelBetween(settings.nightBrightness(), settings.brightness(),
                                    dayShare(elevation));

  if (next != level_) {
    Serial.printf("punkt: auto dim level=%u sun=%.1fdeg\n", next, static_cast<double>(elevation));
  }

  return next;
}
