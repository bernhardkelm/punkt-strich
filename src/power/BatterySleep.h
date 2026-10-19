#pragma once

#include <cstdint>

class Display;
class Settings;
class StateMachine;
class Touch;

/**
 * Deep sleep on battery after a quiet spell, only on boards with a VBUS sense pin. A touch on the
 * left coin wakes the board at once, a timer wakes it every kUsbCheckMs to look for USB power.
 */
namespace BatterySleep {

/** Interval between two evaluations of due(). */
constexpr uint32_t kCheckIntervalMs = 10000;
/** Charge at or below which the board sleeps after kCriticalGraceMs whatever the sessions show. */
constexpr int kCriticalPercent = 3;
/** Time without input after which a critical battery sends the board to sleep. */
constexpr uint32_t kCriticalGraceMs = 60000;
/** Interval of the timer wake that checks whether USB power came back. */
constexpr uint32_t kUsbCheckMs = 30000;

/**
 * Returns to deep sleep at once when the running boot is the timer wake of a battery sleep and
 * USB is still absent, re-arming the touch wake with the threshold the first sleep stored. Call
 * first thing in setup(): the matrix, the LED and Wi-Fi stay untouched on that path. Returns on
 * every other boot and when USB came back, which then boots normally.
 * @returns {void}
 */
void recheckUsb();

/**
 * Returns whether the board should enter the battery sleep now. On battery a critical charge of
 * kCriticalPercent or less is due once no input was seen for kCriticalGraceMs, whatever the
 * setting, the sessions or LOADING say. Otherwise batterySleepMin must be on, no session busy or
 * waiting for a human and no input seen for batterySleepMin minutes, a latched LOADING does not
 * hold the board awake. Input is the state machine's last activity, the boot included, or the
 * moment USB was unplugged. Call on every loop() pass,
 * it is evaluated at most every kCheckIntervalMs and the calls in between return false.
 * @param {uint32_t} nowMs - millis() at the time of the call
 * @param {const Settings&} settings - batterySleepMin minutes, 0 is off
 * @param {const StateMachine&} state - session table and last activity time
 * @returns {bool} - true once the board is due to sleep
 */
bool due(uint32_t nowMs, const Settings& settings, const StateMachine& state);

/**
 * Darkens the matrix and the LED, drops Wi-Fi, arms the left coin as a single touch wake and the
 * USB check timer and enters deep sleep. Never returns, the next boot starts from setup().
 * @param {Display&} display - matrix to put into standby
 * @param {Touch&} touch - left coin electrode, the one pad whose wake threshold is armed
 * @returns {void}
 */
[[noreturn]] void enter(Display& display, Touch& touch);

}
