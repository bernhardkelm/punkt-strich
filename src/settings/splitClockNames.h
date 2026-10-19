#pragma once

#include "settings/Settings.h"

/**
 * Returns the lowercase name of a split clock style.
 * @param {SplitClock} style - split clock style to name
 * @returns {const char*} - "digits", "squares", "stagger" or "bar"
 */
const char* splitClockName(SplitClock style);

/**
 * Parses a lowercase split clock name.
 * @param {const char*} name - candidate name, may be null
 * @param {SplitClock&} out - receives the split clock on success
 * @returns {bool} - true when the name was known
 */
bool parseSplitClock(const char* name, SplitClock& out);
