#pragma once

#include "Display.h"

#include <cstdint>

/** Brightness applied during a glitch burst frame. */
constexpr uint8_t kGlitchBrightness = 15;

/**
 * Fills the pixel masks of the plain 2-pixel thick X.
 * @param {uint8_t (&)[Display::kHeight]} rows - out array, bit x lights column x
 * @returns {void}
 */
void xRows(uint8_t (&rows)[Display::kHeight]);

/**
 * Builds a glitched copy of a frame by shifting random rows and flipping random pixels.
 * @param {const uint8_t (&)[Display::kHeight]} base - frame the burst distorts
 * @param {uint8_t (&)[Display::kHeight]} rows - out array, filled with the glitched pixel masks
 * @returns {void}
 */
void generateGlitchRows(const uint8_t (&base)[Display::kHeight], uint8_t (&rows)[Display::kHeight]);

/**
 * Draws pixel masks at the given brightness, one row at a time.
 * @param {Display&} display - matrix to draw on
 * @param {uint8_t} brightness - brightness level to apply
 * @param {const uint8_t (&)[Display::kHeight]} rows - pixel mask per row, one bit per column
 * @returns {void}
 */
void drawRows(Display& display, uint8_t brightness, const uint8_t (&rows)[Display::kHeight]);
