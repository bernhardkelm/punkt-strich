#pragma once

/**
 * Sets the power state the host VbusSense reports to the battery view.
 * @param {bool} usb - true while USB power counts as present
 * @param {int} percent - charge estimate on battery, 0 to 100
 * @returns {void}
 */
void setHostBattery(bool usb, int percent);
