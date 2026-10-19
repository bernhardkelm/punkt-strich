#pragma once

class AsyncWebServer;
struct Commands;

/**
 * Registers POST /context and POST /usage, the two percentages the status line reports.
 * @param {AsyncWebServer&} server - server the routes are added to
 * @param {Commands&} commands - queue the reported percentages are written to
 * @returns {void}
 */
void registerPercentRoutes(AsyncWebServer& server, Commands& commands);
