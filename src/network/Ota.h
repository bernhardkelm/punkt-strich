#pragma once

#include <cstdint>

/** ArduinoOTA wrapper, the library itself stays inside Ota.cpp. */
namespace Ota {

/** Invoked once when an update starts. */
using StartCallback = void (*)();

/** Invoked for every received chunk so the caller can keep its animation moving. */
using ProgressCallback = void (*)(uint32_t done, uint32_t total);

/** Invoked once when an update fails, the board keeps running the old firmware. */
using ErrorCallback = void (*)();

/**
 * Starts the OTA service under the board name.
 * @param {const char*} name - board name, announced as the OTA hostname
 * @param {StartCallback} onStart - called when an update begins, may be null
 * @param {ProgressCallback} onProgress - called per received chunk, may be null
 * @param {ErrorCallback} onError - called when an update fails, may be null
 * @returns {void}
 */
void begin(const char* name, StartCallback onStart, ProgressCallback onProgress,
           ErrorCallback onError);

/** Services pending OTA packets. Call from loop(). */
void handle();

}
