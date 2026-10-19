#pragma once

#include <cstdint>

/** USB VBUS sensing on boards that wire VBUS to an ADC pin through a 1:2 divider. */
namespace VbusSense {

/**
 * Sets the pin attenuation once, does nothing on boards without a VBUS sense pin.
 * @returns {void}
 */
void begin();

/**
 * Samples the sense pin at most every 200 ms and updates the charge estimate while on battery,
 * call once per loop() iteration. Does nothing on boards without a VBUS sense pin.
 * @param {uint32_t} nowMs - millis() at the time of the call
 * @returns {void}
 */
void tick(uint32_t nowMs);

/**
 * Returns whether the last sample saw USB power. The HTTP task reads this from another FreeRTOS
 * task, a plain store/load is acceptable in this codebase.
 * @returns {bool} - true while the pin read above the USB threshold
 */
bool usbPresent();

/**
 * Returns whether USB was plugged in or unplugged since the last call, and clears the flag.
 * @returns {bool} - true once per change of USB presence
 */
bool consumeUsbChange();

/**
 * Returns the last VBUS voltage, the pin reading times the divider ratio.
 * @returns {int} - millivolts, 0 before the first sample
 */
int vbusMv();

/**
 * Returns the estimated charge of the cell, batteryPercent() of VBUS smoothed over a few seconds
 * of battery samples and held until the estimate clearly moved, so it does not flicker. Read from
 * the HTTP task like usbPresent().
 * @returns {int} - 0 to 100, -1 on USB, before the first battery sample and without a sense pin
 */
int percent();

}
