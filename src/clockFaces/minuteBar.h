#pragma once

#include "Frame.h"

#include <cstdint>

/** Pixels of a full minute bar, two rows of seven */
constexpr uint8_t kMinuteBarMax = 14;

/**
 * Returns the length of the BAR split clock's minute bar, round(minute * 14 / 60) with halves
 * rounded up.
 * @param {uint8_t} minute - 0 to 59, more counts as 59
 * @returns {uint8_t} - 0 to kMinuteBarMax pixels
 */
uint8_t minuteBarPixels(uint8_t minute);

/**
 * Returns the minute bar of the BAR split clock, the first seven pixels on row 6 from column 0 to
 * 6, the rest on row 7 from column 0.
 * @param {uint8_t} pixels - bar length, more than kMinuteBarMax counts as kMinuteBarMax
 * @returns {Frame} - frame with the bar pixels set
 */
Frame minuteBarFrame(uint8_t pixels);
