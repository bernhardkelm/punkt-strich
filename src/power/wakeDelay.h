#pragma once

#include <cstdint>
#include <ctime>

/**
 * Returns the seconds from a local time to the next occurrence of a clock time.
 * @param {const tm&} local - broken-down local time the sleep starts at
 * @param {uint16_t} wakeAt - minutes since midnight the board wakes, 0 to 1439
 * @returns {uint32_t} - seconds until wakeAt, a full day when both times are equal
 */
uint32_t secondsUntilWake(const tm& local, uint16_t wakeAt);
