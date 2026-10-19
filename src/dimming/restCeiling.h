#pragma once

#include <cstdint>

/**
 * Returns the brightness ceiling to apply, capped to the night level while the clock rests.
 * @param {uint8_t} sunLevel - ceiling AutoDim computed, 0 to 15
 * @param {uint8_t} nightLevel - ceiling to cap to while resting, 0 to 15
 * @param {bool} resting - true while RestDim considers the clock resting
 * @returns {uint8_t} - ceiling to apply, 0 to 15
 */
uint8_t restCeiling(uint8_t sunLevel, uint8_t nightLevel, bool resting);
