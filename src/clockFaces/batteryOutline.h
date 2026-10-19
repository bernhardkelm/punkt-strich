#pragma once

#include "Frame.h"

#include <cstdint>

/** First canvas column inside the battery outline */
constexpr uint8_t kBatteryInsideLeft = 1;
/** Columns inside the battery outline, 1 to 13 */
constexpr uint8_t kBatteryInsideWidth = 13;
/** First canvas row inside the battery outline */
constexpr uint8_t kBatteryInsideTop = 1;
/** Rows inside the battery outline, 1 to 6 */
constexpr uint8_t kBatteryInsideHeight = 6;

/**
 * Returns the battery outline over the 16x8 canvas: a one-pixel body over columns 0 to 14 and
 * rows 0 to 7 around the inside, the terminal nub at column 15 on rows 2 to 5.
 * @returns {Frame} - frame with the outline pixels set
 */
Frame batteryOutline();
