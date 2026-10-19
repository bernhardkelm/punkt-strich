#include "dimming/solarElevation.h"

#include <cmath>

namespace {

constexpr float kPi = 3.14159265358979f;
constexpr float kDegPerRad = 180.0f / kPi;
constexpr float kRadPerDeg = kPi / 180.0f;
constexpr float kDaysPerYear = 365.0f;
constexpr float kMinutesPerHour = 60.0f;
constexpr float kMinutesPerDegree = 4.0f;
constexpr float kSecondsPerMinute = 60.0f;

/**
 * Returns the fractional year angle the NOAA series expand around.
 * @param {const tm&} utc - broken-down UTC time
 * @returns {float} - angle in radians, 0 at the start of the year
 */
float fractionalYear(const tm& utc) {
  const float hour = static_cast<float>(utc.tm_hour) + static_cast<float>(utc.tm_min) / 60.0f;
  return 2.0f * kPi / kDaysPerYear * (static_cast<float>(utc.tm_yday) + (hour - 12.0f) / 24.0f);
}

/**
 * Returns the equation of time, the offset between clock time and true solar time.
 * @param {float} gamma - fractional year angle in radians
 * @returns {float} - minutes to add to mean solar time
 */
float equationOfTimeMin(float gamma) {
  return 229.18f * (0.000075f + 0.001868f * cosf(gamma) - 0.032077f * sinf(gamma) -
                    0.014615f * cosf(2.0f * gamma) - 0.040849f * sinf(2.0f * gamma));
}

/**
 * Returns the sun's declination, its latitude on the celestial sphere.
 * @param {float} gamma - fractional year angle in radians
 * @returns {float} - declination in radians
 */
float declinationRad(float gamma) {
  return 0.006918f - 0.399912f * cosf(gamma) + 0.070257f * sinf(gamma) -
         0.006758f * cosf(2.0f * gamma) + 0.000907f * sinf(2.0f * gamma) -
         0.002697f * cosf(3.0f * gamma) + 0.00148f * sinf(3.0f * gamma);
}

}

float solarElevationDeg(time_t utc, float latitudeDeg, float longitudeDeg) {
  tm moment = {};
  gmtime_r(&utc, &moment);

  const float gamma = fractionalYear(moment);
  const float clockMin = static_cast<float>(moment.tm_hour) * kMinutesPerHour +
                         static_cast<float>(moment.tm_min) +
                         static_cast<float>(moment.tm_sec) / kSecondsPerMinute;
  const float trueSolarMin = clockMin + equationOfTimeMin(gamma) + kMinutesPerDegree * longitudeDeg;
  const float hourAngle = (trueSolarMin / kMinutesPerDegree - 180.0f) * kRadPerDeg;
  const float latitude = latitudeDeg * kRadPerDeg;
  const float declination = declinationRad(gamma);
  const float cosZenith = sinf(latitude) * sinf(declination) +
                          cosf(latitude) * cosf(declination) * cosf(hourAngle);
  const float bounded = cosZenith > 1.0f ? 1.0f : (cosZenith < -1.0f ? -1.0f : cosZenith);
  return 90.0f - acosf(bounded) * kDegPerRad;
}
