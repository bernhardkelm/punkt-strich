#pragma once

#include "settings/Settings.h"

/**
 * Returns the lowercase name of an activity.
 * @param {Activity} activity - activity to name
 * @returns {const char*} - "thinking", "reading", "editing", "shell", "delegating" or "mcp"
 */
const char* activityName(Activity activity);

/**
 * Parses a lowercase activity name.
 * @param {const char*} name - candidate name, may be null
 * @param {Activity&} out - receives the activity on success
 * @returns {bool} - true when the name was known
 */
bool parseActivity(const char* name, Activity& out);
