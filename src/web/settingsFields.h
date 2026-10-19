#pragma once

#include <ArduinoJson.h>
#include <cstdint>

/**
 * Reads the optional per activity animations of a request body.
 * @param {JsonVariantConst} value - field value, null when the key is absent
 * @param {int8_t*} out - receives one WorkingAnimation value per activity, kAbsent for the rest
 * @param {const char*&} error - receives the rejection message when the field was refused
 * @returns {bool} - false when the field is present but not an object of known activity names
 */
bool readAnimations(JsonVariantConst value, int8_t* out, const char*& error);

/**
 * Reads the optional clock face of a request body, a board without a clock half refuses it.
 * @param {JsonVariantConst} value - the clockFace member, null when absent
 * @param {int8_t&} out - receives the ClockFace value, left at kAbsent when absent
 * @param {const char*&} error - receives the rejection message when the value is refused
 * @returns {bool} - true when the member was absent or valid
 */
bool readClockFace(JsonVariantConst value, int8_t& out, const char*& error);

/**
 * Reads the optional split clock style of a request body, a board without a clock half refuses it.
 * @param {JsonVariantConst} value - the splitClock member, null when absent
 * @param {int8_t&} out - receives the SplitClock value, left at kAbsent when absent
 * @param {const char*&} error - receives the rejection message when the value is refused
 * @returns {bool} - true when the member was absent or valid
 */
bool readSplitClock(JsonVariantConst value, int8_t& out, const char*& error);

/**
 * Reads the optional battery sleep minutes of a request body, a board without VBUS sensing refuses
 * it.
 * @param {JsonVariantConst} value - the batterySleepMin member, null when absent
 * @param {int32_t&} out - receives the minutes, left at kAbsent when absent
 * @param {const char*&} error - receives the rejection message when the value is refused
 * @returns {bool} - true when the member was absent or valid
 */
bool readBatterySleep(JsonVariantConst value, int32_t& out, const char*& error);
