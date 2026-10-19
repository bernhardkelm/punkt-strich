#pragma once

#include <cstdint>

class Settings;
class StateMachine;
class Touch;
struct Commands;

/** REST API and settings page on port 80, ESPAsyncWebServer itself stays inside HttpServer.cpp. */
namespace HttpServer {

/**
 * Registers every route and starts listening, the handlers run on the AsyncTCP task.
 * @param {Commands&} commands - queue the handlers hand work to loop() through
 * @param {Settings&} settings - settings the handlers read and write
 * @param {const StateMachine&} state - state machine the handlers read the current state from
 * @param {const Touch&} touch - punkt's coin or strich's left coin, the status route reports it
 * @param {const Touch&} touchRight - strich's right coin, only reported on strich
 * @returns {void}
 */
void begin(Commands& commands, Settings& settings, const StateMachine& state, const Touch& touch,
           const Touch& touchRight);

/**
 * Checks the listening socket every few seconds and starts it again while it is down, a failed
 * bind would otherwise leave the board without its API until a reboot. Does nothing before begin().
 * @param {uint32_t} nowMs - millis() at the time of the call
 * @returns {void}
 */
void tick(uint32_t nowMs);

}
