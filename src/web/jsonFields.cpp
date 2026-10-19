#include "web/jsonFields.h"

#include "settings/Settings.h"
#include "settings/cities.h"
#include "settings/clockMinutes.h"
#include "settings/workingAnimationNames.h"

#include <cstring>

namespace {

/**
 * Returns whether a character is allowed anywhere inside a board name.
 * @param {char} character - character to check
 * @returns {bool} - true for a lowercase letter, a digit or a hyphen
 */
bool isNameCharacter(char character) {
  const bool letter = character >= 'a' && character <= 'z';
  const bool digit = character >= '0' && character <= '9';
  return letter || digit || character == '-';
}

}

bool readNumber(JsonVariantConst value, int32_t max, int32_t& out) {
  if (value.isNull()) {
    return true;
  }

  if (!value.is<int32_t>()) {
    return false;
  }

  const int32_t number = value.as<int32_t>();

  if (number < 0 || number > max) {
    return false;
  }

  out = number;
  return true;
}

bool readFlag(JsonVariantConst value, int8_t& out) {
  if (value.isNull()) {
    return true;
  }

  if (!value.is<bool>()) {
    return false;
  }

  out = value.as<bool>() ? 1 : 0;
  return true;
}

bool readText(JsonVariantConst value, size_t maxLength, const char*& out) {
  out = nullptr;

  if (value.isNull()) {
    return true;
  }

  if (!value.is<const char*>()) {
    return false;
  }

  const char* text = value.as<const char*>();

  if (strlen(text) > maxLength) {
    return false;
  }

  out = text;
  return true;
}

bool readName(JsonVariantConst value, const char*& out) {
  out = nullptr;

  if (value.isNull()) {
    return true;
  }

  if (!value.is<const char*>()) {
    return false;
  }

  const char* text = value.as<const char*>();
  const size_t length = strlen(text);

  if (length == 0 || length > Settings::kNameMax) {
    return false;
  }

  if (text[0] == '-' || text[length - 1] == '-') {
    return false;
  }

  for (size_t i = 0; i < length; i++) {
    if (!isNameCharacter(text[i])) {
      return false;
    }
  }

  out = text;
  return true;
}

bool readClock(JsonVariantConst value, int32_t& out) {
  if (value.isNull()) {
    return true;
  }

  if (!value.is<const char*>()) {
    return false;
  }

  uint16_t minutes = 0;

  if (!parseClockMinutes(value.as<const char*>(), minutes)) {
    return false;
  }

  out = minutes;
  return true;
}

bool readCity(JsonVariantConst value, const char*& out) {
  out = nullptr;

  if (value.isNull()) {
    return true;
  }

  if (!value.is<const char*>()) {
    return false;
  }

  const char* name = value.as<const char*>();

  if (findCity(name) == nullptr) {
    return false;
  }

  out = name;
  return true;
}

bool readWorkingAnimation(JsonVariantConst value, int8_t& out) {
  out = -1;

  if (value.isNull()) {
    return true;
  }

  const char* name = value.is<const char*>() ? value.as<const char*>() : nullptr;
  WorkingAnimation animation = WorkingAnimation::TETRIS;

  if (!parseWorkingAnimation(name, animation)) {
    return false;
  }

  out = static_cast<int8_t>(animation);
  return true;
}
