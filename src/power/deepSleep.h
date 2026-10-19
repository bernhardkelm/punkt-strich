#pragma once

#include <cstdint>

class Display;

/** Deep sleep the board entered, kept in RTC memory so the next boot knows which one it left. */
enum class SleepKind : uint8_t { NONE, NIGHT, BATTERY };

/**
 * Logs the wake source and the sleep the running boot left, call right after Serial.begin().
 * Logs nothing after a reset that was no touch or timer wake.
 * @returns {void}
 */
void logSleepWake();

/**
 * Returns the kind of deep sleep the running boot woke from.
 * @returns {SleepKind} - NIGHT or BATTERY after a deep sleep wake, NONE after every other reset
 */
SleepKind wokeFrom();

/**
 * Returns the touch wake threshold the last deep sleep armed, so the wake can be armed again
 * without the calibration Touch::begin() runs.
 * @returns {uint32_t} - raw delta above the sleep benchmark, 0 when the boot was no deep sleep wake
 */
uint32_t storedWakeThreshold();

/**
 * Puts the matrix into standby, turns the LED off and drops Wi-Fi, which leaves the board dark
 * for a deep sleep.
 * @param {Display&} display - matrix to put into standby
 * @returns {void}
 */
void darkenForSleep(Display& display);

/**
 * Stores the sleep kind and the touch wake threshold in RTC memory, arms the pad on pin as the
 * only touch channel scanned while asleep and enters deep sleep. A timer wake must be armed
 * before the call. Never returns, the next boot starts from setup().
 * @param {SleepKind} kind - sleep the next boot reports through wokeFrom()
 * @param {uint8_t} pin - GPIO of the wake pad, the left coin
 * @param {uint32_t} threshold - raw delta above the sleep benchmark that wakes the chip
 * @returns {void}
 */
[[noreturn]] void startDeepSleep(SleepKind kind, uint8_t pin, uint32_t threshold);
