#pragma once

#include "settings/Settings.h"

/**
 * Returns the lowercase name of a working animation.
 * @param {WorkingAnimation} animation - animation to name
 * @returns {const char*} - "tetris", "life", "knight", "orbit", "code", "rain", "scan", "pipes",
 * "pointer", "pong" or "terminal"
 */
const char* workingAnimationName(WorkingAnimation animation);

/**
 * Parses a lowercase working animation name.
 * @param {const char*} name - candidate name, may be null
 * @param {WorkingAnimation&} out - receives the animation on success
 * @returns {bool} - true when the name was known
 */
bool parseWorkingAnimation(const char* name, WorkingAnimation& out);
