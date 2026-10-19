#pragma once

#include <WString.h>
#include <cstdint>

/** WiFiManager wrapper, the library itself stays inside WifiSetup.cpp. */
namespace WifiSetup {

/** What the station link is doing, the matrix shows a picture of its own for PORTAL and LOST. */
enum class LinkState : uint8_t { UP, CONNECTING, PORTAL, LOST };

/** Invoked right before the restart a portal save triggers, the caller still owns the matrix. */
using SavedCallback = void (*)();

/**
 * Sets the hostname, enables auto reconnect and connects with the stored credentials, a board
 * without any starts the non-blocking config portal instead. Power::apply owns the sleep mode.
 * The name is copied, it is read once and never re-read. Blocks up to 5 s while autoConnect()
 * waits for the connect with stored credentials, a slower connect is picked up by process().
 * @param {const char*} name - board name, the hostname, the mDNS host and the portal SSID stem
 * @returns {void}
 */
void begin(const char* name);

/**
 * Services the config portal, detects the first station connection and re-associates after the
 * link has been down for ten seconds. Stored credentials that have not connected 15 s after boot
 * re-associate on the same cadence until they do, the portal never opens for them. A connection
 * that came out of a portal save restarts the chip instead, the services start on the next boot.
 * @param {uint32_t} nowMs - millis() at the time of the call
 * @param {SavedCallback} onSaved - called before the restart a portal save triggers, may be null
 * @returns {bool} - true exactly once, on the first connection
 */
bool process(uint32_t nowMs, SavedCallback onSaved);

/**
 * Returns what the station link is doing, cheap enough to call from every loop iteration.
 * @param {uint32_t} nowMs - millis() at the time of the call
 * @returns {LinkState} - PORTAL while the portal runs without stored credentials, CONNECTING
 * until the first connection, LOST once the boot connect timed out or the watchdog started
 * reconnecting, UP while the station is associated
 */
LinkState link(uint32_t nowMs);

/**
 * Returns the station address.
 * @returns {String} - dotted quad, "0.0.0.0" while disconnected
 */
String ipString();

/**
 * Returns the signal strength of the current association.
 * @returns {int8_t} - RSSI in dBm, 0 while disconnected
 */
int8_t rssi();

/** Erases the stored credentials, the caller restarts afterwards. */
void forget();

}
