#pragma once

#include <cstdint>

/**
 * Estimates the charge of the cell from the VBUS voltage strich reads on battery, by linear
 * interpolation between points measured on one cell, so the result is rough.
 * @param {int} vbusMv - VBUS in millivolts while running on battery
 * @returns {uint8_t} - 0 to 100, 100 from 3300 mV up and 0 from 2600 mV down
 */
uint8_t batteryPercent(int vbusMv);
