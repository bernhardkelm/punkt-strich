#include "settings/clockMinutes.h"

#include <cstdio>

namespace {

constexpr uint16_t kMinutesPerHour = 60;
constexpr uint16_t kHoursPerDay = 24;
constexpr uint16_t kMinutesPerDay = kMinutesPerHour * kHoursPerDay;

/**
 * Returns whether a character is a decimal digit.
 * @param {char} c - character to test
 * @returns {bool} - true for 0 to 9
 */
bool isDigit(char c) {
  return c >= '0' && c <= '9';
}

/**
 * Reads two decimal digits as a number.
 * @param {const char*} text - points at the first digit, both already checked
 * @returns {uint16_t} - 0 to 99
 */
uint16_t twoDigits(const char* text) {
  return static_cast<uint16_t>((text[0] - '0') * 10 + (text[1] - '0'));
}

}

bool parseClockMinutes(const char* text, uint16_t& out) {
  if (text == nullptr) {
    return false;
  }

  const bool shaped = isDigit(text[0]) && isDigit(text[1]) && text[2] == ':' && isDigit(text[3]) &&
                      isDigit(text[4]) && text[5] == '\0';

  if (!shaped) {
    return false;
  }

  const uint16_t hours = twoDigits(text);
  const uint16_t minutes = twoDigits(text + 3);

  if (hours >= kHoursPerDay || minutes >= kMinutesPerHour) {
    return false;
  }

  out = static_cast<uint16_t>(hours * kMinutesPerHour + minutes);
  return true;
}

void formatClockMinutes(uint16_t minutes, char* out, size_t size) {
  const uint16_t wrapped = minutes % kMinutesPerDay;
  snprintf(out, size, "%02u:%02u", wrapped / kMinutesPerHour, wrapped % kMinutesPerHour);
}
