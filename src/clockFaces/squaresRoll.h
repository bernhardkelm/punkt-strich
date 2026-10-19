#pragma once

#include <cstdint>

/**
 * Returns the time the SQUARES split clock shows next on its way to a new time, one pixel per
 * step. Within the same tens of the same hour the new time shows at once. A tens change first
 * drains the ones square from the top, an hour change then drains the tens square as well, and
 * the new time shows once both are empty, so every step lands on a real minute.
 * @param {uint16_t} shown - minute of the day the squares show, 0 to 1439
 * @param {uint16_t} target - minute of the day to reach, 0 to 1439
 * @returns {uint16_t} - minute of the day after the step, target once nothing is left to drain
 */
uint16_t squaresRollStep(uint16_t shown, uint16_t target);
