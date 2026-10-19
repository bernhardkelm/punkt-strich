#pragma once

class AsyncWebServer;

/**
 * Registers GET /cities, which lists every city the board can be placed in with its region and
 * coordinates, so the settings page builds its picker from the same table the firmware uses.
 * @param {AsyncWebServer&} server - server to add the route to
 * @returns {void}
 */
void registerCityRoutes(AsyncWebServer& server);
