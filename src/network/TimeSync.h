#pragma once

#include <cstdint>
#include <ctime>

/** SNTP wrapper, owns the POSIX timezone and reports when the first sync landed. */
namespace TimeSync {

/**
 * Starts SNTP against the public pools and applies the timezone, call once the station is up.
 * Repeated calls are ignored, setTimezone() changes the zone afterwards.
 * @param {const char*} tz - POSIX TZ string, for example CET-1CEST,M3.5.0,M10.5.0/3
 * @returns {void}
 */
void begin(const char* tz);

/**
 * Applies a timezone, visible to the next localTime() call. Safe before begin(), which a board
 * waking from deep sleep without Wi-Fi never reaches.
 * @param {const char*} tz - POSIX TZ string
 * @returns {void}
 */
void setTimezone(const char* tz);

/**
 * Returns whether the clock carries a synced time, never blocks.
 * @returns {bool} - true once the first sync landed or the RTC kept the time through a sleep
 */
bool synced();

/**
 * Fills the broken-down local time, never blocks.
 * @param {tm&} out - receives the local time, untouched while unsynced
 * @returns {bool} - true when a synced time was written
 */
bool localTime(tm& out);

/**
 * Polls for the first sync and logs it once.
 * @param {uint32_t} nowMs - millis() at the time of the call
 * @returns {void}
 */
void tick(uint32_t nowMs);

}
