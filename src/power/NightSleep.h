#pragma once

#include <cstdint>

class Display;
class Settings;
class StateMachine;
class Touch;

/** Deep sleep inside the night window while nightOff and nightSleep are on, the touch pad wakes. */
namespace NightSleep {

/** Time the board stays up after the last input, a touch wake included. */
constexpr uint32_t kGraceMs = 600000;
/** Interval between two evaluations of due(). */
constexpr uint32_t kCheckIntervalMs = 10000;
/** Time after a touch wake in which a second tap must arrive, otherwise the board sleeps again. */
constexpr uint32_t kWakeConfirmMs = 2000;

/**
 * Returns whether the running boot was caused by a touch on the sleep pad during the night
 * sleep, the wake that waits for a confirming tap. A touch out of the battery sleep does not
 * count, it boots without one.
 * @returns {bool} - true when the wake source was the touch peripheral and the sleep the night one
 */
bool wokeByTouch();

/**
 * Waits for a second tap that confirms a touch wake and re-enters deep sleep when none arrives
 * within kWakeConfirmMs. Call after touch.begin() and before anything draws, the matrix stays
 * dark on both paths. The finger that caused the wake may still rest on the pad while
 * Touch::begin() calibrates, which leaves the baseline too high until the release snaps it down,
 * so the confirming tap is the one after that calibration.
 * @param {Touch&} touch - electrode polled for the confirming tap
 * @param {Display&} display - matrix handed to enter() when the window passes unconfirmed
 * @param {const Settings&} settings - wake settings handed to enter() on the unconfirmed path
 * @returns {bool} - true once a tap confirmed the wake, otherwise the call does not return
 */
bool confirmWake(Touch& touch, Display& display, const Settings& settings);

/**
 * Returns whether the board should enter deep sleep now: nightOff and nightSleep are both on, no
 * named session is open, LOADING is not latched, the synced local time lies inside the night
 * window and no input was seen for kGraceMs. Evaluated at most every kCheckIntervalMs, the calls
 * in between return false.
 * @param {uint32_t} nowMs - millis() at the time of the call
 * @param {const Settings&} settings - nightOff and nightSleep flags, sleepAt and wakeAt
 * @param {const StateMachine&} state - session table, loading flag and last activity time
 * @returns {bool} - true once the board is due to sleep
 */
bool due(uint32_t nowMs, const Settings& settings, const StateMachine& state);

/**
 * Darkens the matrix and the LED, drops Wi-Fi, arms the touch wake and enters deep sleep. With a
 * synced clock the wake timer is armed for the next wakeAt as well. Never returns, the next boot
 * starts from setup().
 * @param {Display&} display - matrix to put into standby
 * @param {Touch&} touch - left coin electrode, the one pad whose wake threshold is armed
 * @param {const Settings&} settings - wakeAt time the timer wake is armed for
 * @returns {void}
 */
[[noreturn]] void enter(Display& display, Touch& touch, const Settings& settings);

}
