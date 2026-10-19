#pragma once

#include <cstdint>

/** Counts power cycles that follow each other quickly, the trigger for the factory reset. */
namespace BootCounter {

/** Number of fast power cycles that request a factory reset. */
constexpr uint8_t kResetCount = 3;

/**
 * Logs the reset reason, then increments and persists the boot count when the boot followed a
 * power-on or an external pin reset. Brownout, watchdog, panic, software, USB and deep sleep
 * boots leave the stored count alone.
 * @returns {uint8_t} - count including the current boot, 1 for a boot that is not counted
 */
uint8_t begin();

/**
 * Returns the reason of the last reset as a short lowercase name for logs and GET /state.
 * @returns {const char*} - "poweron", "external", "software", "panic", "int-wdt", "task-wdt",
 * "wdt", "deepsleep", "brownout", "sdio", "usb" or "unknown"
 */
const char* resetReason();

/**
 * Clears the count once the device stayed up past the window. Call from loop().
 * @param {uint32_t} nowMs - millis() at the time of the call
 * @returns {void}
 */
void tick(uint32_t nowMs);

/** Clears the stored count immediately. */
void reset();

}
