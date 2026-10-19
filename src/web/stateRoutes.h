#pragma once

class AsyncWebServer;
class Settings;
class StateMachine;
class Touch;
struct Commands;

/**
 * Registers GET and POST /state.
 * @param {AsyncWebServer&} server - server the routes are added to
 * @param {Commands&} commands - queue a requested state is written to
 * @param {Settings&} settings - settings the status body reports
 * @param {const StateMachine&} state - state machine the current state is read from
 * @param {const Touch&} touch - punkt's coin or strich's left coin, the status body reports it
 * @param {const Touch&} touchRight - strich's right coin, only reported on strich
 * @returns {void}
 */
void registerStateRoutes(AsyncWebServer& server, Commands& commands, Settings& settings,
                         const StateMachine& state, const Touch& touch, const Touch& touchRight);
