#pragma once

#include "Frame.h"

#include <cstdint>

/**
 * Returns the inside of the battery outline lit from the left in proportion to a charge, the
 * number of columns rounded to the nearest one.
 * @param {uint8_t} percent - charge 0 to 100, more counts as 100
 * @returns {Frame} - frame with the fill pixels set on every inside row
 */
Frame batteryFill(uint8_t percent);
