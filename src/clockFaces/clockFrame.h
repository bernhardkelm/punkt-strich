#pragma once

#include "Display.h"
#include "animations/digitFont.h"
#include "settings/Settings.h"

#include <cstdint>

/** Columns the minute pair moves left between the full canvas face and the punkt clock layout */
constexpr uint8_t kMinuteShiftMax = 8;
/** Digit height of the Staircase face and the punkt clock layout, the plain glyph */
constexpr uint8_t kStaircaseRows = kDigitRows;
/** Digit height of the Tall face, the full canvas height */
constexpr uint8_t kTallRows = Display::kHeight;
/** Canvas column of the minute tens on the full faces, the minute ones sit 4 columns right */
constexpr int8_t kFaceMinuteX = 9;

/**
 * Returns the digit height a clock face draws the full canvas time with.
 * @param {ClockFace} face - clock face to look up
 * @returns {uint8_t} - kTallRows for TALL, kStaircaseRows otherwise
 */
uint8_t clockFaceRows(ClockFace face);

/**
 * Clears the canvas and draws a 24-hour time on it, the hour pair at columns 0 and 4 hanging from
 * row 0, the minute pair at columns 9 and 13 moved left by the shift and standing on row 7.
 * @param {Display&} display - canvas to draw on, pushed by the caller
 * @param {uint8_t} hour - 0 to 23, shown with a leading zero
 * @param {uint8_t} minute - 0 to 59, shown with a leading zero
 * @param {uint8_t} minuteShift - columns the minute pair moves left, 0 to kMinuteShiftMax
 * @param {uint8_t} hourRows - hour digit height, kStaircaseRows to kTallRows
 * @param {uint8_t} minuteRows - minute digit height, kStaircaseRows to kTallRows
 * @returns {void}
 */
void drawClockFrame(Display& display, uint8_t hour, uint8_t minute, uint8_t minuteShift,
                    uint8_t hourRows, uint8_t minuteRows);
