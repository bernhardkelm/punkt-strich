#pragma once

#include "Frame.h"
#include "settings/Settings.h"

#include <cstdint>

/** Top row of the 3x3 minutes of the DIGITS split clock */
constexpr int8_t kSmallMinutesY = 5;

/**
 * Returns the hour pair of the split clock, 3x4 digits at columns 0 and 4 on rows 0 to 3, the
 * same in every style.
 * @param {uint8_t} hour - 0 to 23, shown with a leading zero
 * @returns {Frame} - frame with the hour pixels set
 */
Frame splitHoursFrame(uint8_t hour);

/**
 * Returns the picture of the split clock in the left half, the hours over the minutes as the
 * style shows them: 3x3 digits on rows 5 to 7 (DIGITS), two counting squares on rows 5 to 7
 * (SQUARES), 3x4 digits at columns 1 and 5 on rows 4 to 7 (STAGGER) or a bar on rows 6 and 7
 * (BAR). Column 7 stays dark in every style but STAGGER.
 * @param {SplitClock} style - split clock style
 * @param {uint8_t} hour - 0 to 23
 * @param {uint8_t} minute - 0 to 59
 * @returns {Frame} - frame with the clock pixels set, bit x of a row is column x of the left half
 */
Frame splitClockFrame(SplitClock style, uint8_t hour, uint8_t minute);
