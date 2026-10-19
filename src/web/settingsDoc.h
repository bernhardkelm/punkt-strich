#pragma once

#include <ArduinoJson.h>

class Settings;
struct SettingsPatch;

/**
 * Builds the full settings object from the cached values, overlaid with a patch loop() has not
 * applied yet. The patch has to outlive the serialisation, its name and city are held by
 * pointer.
 * @param {const Settings&} settings - settings store the cached values are read from
 * @param {const SettingsPatch&} patch - validated fields, an empty patch reports the cached values
 * @returns {JsonDocument} - settings object
 */
JsonDocument settingsDoc(const Settings& settings, const SettingsPatch& patch);
