#pragma once

#include <cstdint>
#include <esp_wifi_types.h>

/** CPU frequency and Wi-Fi power save scaling, the driver calls stay inside Power.cpp. */
namespace Power {

/**
 * Returns a short label for a Wi-Fi power save mode.
 * @param {wifi_ps_type_t} mode - mode as the driver reports it
 * @returns {const char*} - "none", "min-modem", "max-modem" or "unknown"
 */
const char* sleepLabel(wifi_ps_type_t mode);

/**
 * Scales the CPU frequency and Wi-Fi power save mode together. Safe to call before Wi-Fi is
 * initialised, the sleep mode is then only stored by the core and applied at driver init.
 * @param {bool} powerSave - true selects 80 MHz and min-modem sleep, false selects 240 MHz and no sleep
 * @returns {void}
 */
void apply(bool powerSave);

/**
 * Samples the chip temperature once a minute, call once per loop() iteration.
 * @param {uint32_t} nowMs - millis() at the time of the call
 * @returns {void}
 */
void tick(uint32_t nowMs);

/**
 * Returns the last chip temperature reading. The HTTP task reads this from another FreeRTOS
 * task, a plain float store/load is acceptable in this codebase.
 * @returns {float} - degrees Celsius, NAN before the first reading
 */
float temperatureC();

}
