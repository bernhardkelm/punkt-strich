#pragma once

#include <ArduinoJson.h>

struct SettingsPatch;

/**
 * Validates every field of a request body into a patch, so an invalid body changes nothing.
 * @param {JsonVariant&} json - parsed request body
 * @param {SettingsPatch&} patch - receives the fields the body carried
 * @param {const char*&} error - receives the rejection message when a field was refused
 * @returns {bool} - true when every field was accepted
 */
bool readSettingsPatch(JsonVariant& json, SettingsPatch& patch, const char*& error);
