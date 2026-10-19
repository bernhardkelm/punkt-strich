#pragma once

class AsyncWebServer;
class StateMachine;
struct Commands;

/**
 * Registers GET and POST /preview, which lists and plays the animations the state machine owns.
 * @param {AsyncWebServer&} server - server to add the routes to
 * @param {Commands&} commands - queue the preview request is written to
 * @param {const StateMachine&} state - state machine the animation list and index come from
 * @returns {void}
 */
void registerPreviewRoutes(AsyncWebServer& server, Commands& commands, const StateMachine& state);
