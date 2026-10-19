#pragma once

#include <ArduinoJson.h>
#include <cstddef>
#include <cstdint>

/** Rejection message of readWorkingAnimation, the only place the accepted names are spelled out. */
constexpr const char* kWorkingAnimationError =
    "animation must be tetris, life, knight, orbit, code, rain, scan, pipes, pointer, pong or "
    "terminal";

/** Rejection message of readCity, the list itself is served by GET /cities. */
constexpr const char* kCityError = "city must be one of the names GET /cities lists";

/** Rejection message of readName, the only place the hostname rule is spelled out. */
constexpr const char* kNameError =
    "name must be 1 to 24 lowercase letters, digits or hyphens, not starting or ending with a "
    "hyphen";

/**
 * Reads an optional whole number field and checks its range.
 * @param {JsonVariantConst} value - field value, null when the key is absent
 * @param {int32_t} max - highest accepted value
 * @param {int32_t&} out - receives the value, stays untouched when the key is absent
 * @returns {bool} - false when the key is present but not a whole number within range
 */
bool readNumber(JsonVariantConst value, int32_t max, int32_t& out);

/**
 * Reads an optional boolean field.
 * @param {JsonVariantConst} value - field value, null when the key is absent
 * @param {int8_t&} out - receives 0 or 1, stays untouched when the key is absent
 * @returns {bool} - false when the key is present but not a boolean
 */
bool readFlag(JsonVariantConst value, int8_t& out);

/**
 * Reads an optional string field and checks its length.
 * @param {JsonVariantConst} value - field value, null when the key is absent
 * @param {size_t} maxLength - longest accepted string
 * @param {const char*&} out - receives the string, stays nullptr when the key is absent
 * @returns {bool} - false when the key is present but not a string within the length
 */
bool readText(JsonVariantConst value, size_t maxLength, const char*& out);

/**
 * Reads an optional board name field and checks it against the hostname rule.
 * @param {JsonVariantConst} value - field value, null when the key is absent
 * @param {const char*&} out - receives the name, stays nullptr when the key is absent
 * @returns {bool} - false when the key is present but not a name of 1 to 24 lowercase letters,
 * digits or hyphens without a leading or trailing hyphen
 */
bool readName(JsonVariantConst value, const char*& out);

/**
 * Reads an optional clock time field of the form HH:MM.
 * @param {JsonVariantConst} value - field value, null when the key is absent
 * @param {int32_t&} out - receives minutes since midnight, stays untouched when the key is absent
 * @returns {bool} - false when the key is present but not a string of the form HH:MM
 */
bool readClock(JsonVariantConst value, int32_t& out);

/**
 * Reads an optional city field and checks it against the city table.
 * @param {JsonVariantConst} value - field value, null when the key is absent
 * @param {const char*&} out - receives the IANA name, stays nullptr when the key is absent
 * @returns {bool} - false when the key is present but not the name of a listed city
 */
bool readCity(JsonVariantConst value, const char*& out);

/**
 * Reads an optional working animation name, used by the animation override on POST /state and by
 * the per activity animations of POST /settings.
 * @param {JsonVariantConst} value - field value, null when the key is absent
 * @param {int8_t&} out - receives the WorkingAnimation value, -1 when the key is absent
 * @returns {bool} - false when the key is present but not a known animation name
 */
bool readWorkingAnimation(JsonVariantConst value, int8_t& out);
