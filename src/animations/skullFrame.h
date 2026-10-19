#pragma once

#include "Display.h"

#include <cstdint>

/**
 * Fills the pixel masks of the plain skull.
 * @param {uint8_t (&)[Display::kHeight]} rows - out array, bit x lights column x
 * @returns {void}
 */
void skullRows(uint8_t (&rows)[Display::kHeight]);
