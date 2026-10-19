#pragma once

#include "Frame.h"

#include <cstdint>

/** Highest charge the digits show, a full cell reads 99 */
constexpr uint8_t kBatteryMaxShownPercent = 99;

/**
 * Returns a charge as 3x4 digits centred inside the battery outline, two digits one column apart
 * and a charge below 10 as the ones digit alone.
 * @param {uint8_t} percent - charge 0 to 100, shown as kBatteryMaxShownPercent at most
 * @returns {Frame} - frame with the digit pixels set
 */
Frame batteryDigits(uint8_t percent);
