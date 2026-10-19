#pragma once

#include <cstdint>

class Settings;
class StateMachine;

/** Schedules the view off and back on across the night window, independent from deep sleep. */
namespace NightOff {

/** Interval between two evaluations of tick(). */
constexpr uint32_t kCheckIntervalMs = 10000;

/**
 * Turns the view off at the start of the night window and back on at its end, evaluated at most
 * every kCheckIntervalMs. With a synced clock and nightOff on, the view goes off once the local
 * time enters the window and the board has been idle for NightSleep::kGraceMs, unless it is
 * already off for another reason. The view returns once the window closes or nightOff turns off,
 * but only while the current off reason is the schedule one, a manual or timeout off is left
 * alone. An unsynced clock changes nothing.
 * @param {uint32_t} nowMs - millis() at the time of the call
 * @param {const Settings&} settings - nightOff flag and the times bounding the window
 * @param {StateMachine&} state - off reason and last activity time, toggled by this call
 * @returns {void}
 */
void tick(uint32_t nowMs, const Settings& settings, StateMachine& state);

}
