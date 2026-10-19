#pragma once

#include <cstdint>

/**
 * Returns whether a time of day lies inside a window that opens at one clock time and closes at
 * another, up to but excluding the closing time, wrapping past midnight.
 * @param {uint16_t} nowMin - minutes since midnight to test, 0 to 1439
 * @param {uint16_t} fromMin - minutes since midnight the window opens
 * @param {uint16_t} toMin - minutes since midnight the window closes, excluded
 * @returns {bool} - true while nowMin lies inside the window, equal fromMin and toMin never match
 */
bool insideNightWindow(uint16_t nowMin, uint16_t fromMin, uint16_t toMin);
