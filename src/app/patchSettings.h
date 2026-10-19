#pragma once

class Settings;
struct SettingsPatch;

/**
 * Writes one settings patch the HTTP task validated, every setter persists its value to NVS.
 * @param {Settings&} settings - settings store the fields are written to
 * @param {const SettingsPatch&} patch - fields the request carried, absent ones are left alone
 * @returns {void}
 */
void patchSettings(Settings& settings, const SettingsPatch& patch);
