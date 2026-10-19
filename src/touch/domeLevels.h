#pragma once

#include <cstdint>

/**
 * Moves a coin's dome level with the entry strengths 0.13, 0.30 and 0.55 for the levels 1 to 3,
 * a level is left again once the strength falls below 70 percent of its entry.
 * @param {uint8_t} level - current level 0 to 3
 * @param {float} strength - smoothed delta as a share of the full touch delta
 * @returns {uint8_t} - new level 0 to 3
 */
uint8_t nextDomeLevel(uint8_t level, float strength);

/**
 * Returns the radius of a coin's dome at a level, in doubled units around the coin centre.
 * @param {uint8_t} level - dome level 0 to 3, higher values read as 3
 * @returns {float} - square root of the level's squared radius, 0 without a dome
 */
float domeRadius(uint8_t level);
