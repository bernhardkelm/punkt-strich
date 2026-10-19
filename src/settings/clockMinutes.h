#pragma once

#include <cstddef>
#include <cstdint>

/** Length of a "HH:MM" clock text including the terminator. */
constexpr size_t kClockTextSize = 6;

/**
 * Parses a clock text of the exact form HH:MM into minutes since midnight.
 * @param {const char*} text - two digit hours, a colon and two digit minutes, 00:00 to 23:59
 * @param {uint16_t&} out - receives the minutes, untouched when the text is rejected
 * @returns {bool} - false for a null pointer, another shape or a value out of range
 */
bool parseClockMinutes(const char* text, uint16_t& out);

/**
 * Formats minutes since midnight as HH:MM with leading zeros.
 * @param {uint16_t} minutes - 0 to 1439, larger values wrap at midnight
 * @param {char*} out - receives the text
 * @param {size_t} size - capacity of out, at least kClockTextSize for the full text
 * @returns {void}
 */
void formatClockMinutes(uint16_t minutes, char* out, size_t size);
