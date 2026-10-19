#pragma once

#include <cstdint>

/**
 * Returns the frame row bits of a run of adjacent canvas columns.
 * @param {uint8_t} first - leftmost column of the run
 * @param {uint8_t} count - columns in the run, 0 returns no bits
 * @returns {uint16_t} - row bits, bit x is canvas column x
 */
constexpr uint16_t columnRun(uint8_t first, uint8_t count) {
  return static_cast<uint16_t>(((1UL << count) - 1UL) << first);
}
