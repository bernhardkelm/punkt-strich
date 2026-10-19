#pragma once

#include "settings/Settings.h"

/**
 * Returns the lowercase name of a clock face.
 * @param {ClockFace} face - clock face to name
 * @returns {const char*} - "staircase" or "tall"
 */
const char* clockFaceName(ClockFace face);

/**
 * Parses a lowercase clock face name.
 * @param {const char*} name - candidate name, may be null
 * @param {ClockFace&} out - receives the clock face on success
 * @returns {bool} - true when the name was known
 */
bool parseClockFace(const char* name, ClockFace& out);
