#pragma once

class AsyncWebServer;
class StateMachine;
struct Commands;

/**
 * Registers POST /view, which switches between the clock, normal, usage and off views.
 * @param {AsyncWebServer&} server - server to add the route to
 * @param {Commands&} commands - queue the view request is written to
 * @param {const StateMachine&} state - state machine asked whether the view change is allowed
 * @returns {void}
 */
void registerViewRoutes(AsyncWebServer& server, Commands& commands, const StateMachine& state);
