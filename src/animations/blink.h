#pragma once

#include <cstdint>

/**
 * Returns whether a period-based blink is lit during the first part of each period.
 * @param {uint32_t} phase - milliseconds since the blink started, not pre-wrapped to the period
 * @param {uint32_t} periodMs - full period length in milliseconds
 * @param {uint32_t} onMs - portion of the period, from its start, that is lit
 * @returns {bool} - true while lit
 */
bool blinkLitFirst(uint32_t phase, uint32_t periodMs, uint32_t onMs);

/**
 * Returns whether a period-based blink is lit during the last part of each period.
 * @param {uint32_t} phase - milliseconds since the blink started, not pre-wrapped to the period
 * @param {uint32_t} periodMs - full period length in milliseconds
 * @param {uint32_t} onMs - portion of the period, at its end, that is lit
 * @returns {bool} - true while lit
 */
bool blinkLitLast(uint32_t phase, uint32_t periodMs, uint32_t onMs);
