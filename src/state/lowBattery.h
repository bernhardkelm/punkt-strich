#pragma once

/**
 * Returns whether the battery counts as low after a new reading: below 15 percent on battery it
 * turns low, at 18 percent or more and on USB it clears, in between and while no estimate exists
 * it keeps its state.
 * @param {bool} wasLow - whether the battery counted as low before the reading
 * @param {bool} usb - USB power present
 * @param {int} percent - estimated charge 0 to 100, -1 while no estimate exists
 * @returns {bool} - true while the battery counts as low
 */
bool lowBatteryAfter(bool wasLow, bool usb, int percent);
