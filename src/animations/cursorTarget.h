#pragma once

#include <cstdint>

/**
 * Returns the step one coordinate takes towards another.
 * @param {int8_t} from - current coordinate
 * @param {int8_t} to - coordinate to reach
 * @returns {int8_t} - -1, 0 or 1
 */
int8_t stepTowards(int8_t from, int8_t to);

/**
 * Picks a random spot the whole cursor L fits on, away from where it stands.
 * @param {int8_t} x - current tip column
 * @param {int8_t} y - current tip row
 * @param {int8_t&} targetX - receives the target column
 * @param {int8_t&} targetY - receives the target row
 * @returns {void}
 */
void pickCursorTarget(int8_t x, int8_t y, int8_t& targetX, int8_t& targetY);

/**
 * Picks the corner a drag opens towards, far enough on both axes for a rectangle to show and with
 * room for the whole L, falling back to the nearest valid corner when the draws keep missing.
 * @param {int8_t} x - arrival tip column
 * @param {int8_t} y - arrival tip row
 * @param {int8_t&} cornerX - receives the corner column
 * @param {int8_t&} cornerY - receives the corner row
 * @returns {void}
 */
void pickDragCorner(int8_t x, int8_t y, int8_t& cornerX, int8_t& cornerY);
