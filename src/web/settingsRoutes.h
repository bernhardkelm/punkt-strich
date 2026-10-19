#pragma once

class AsyncWebServer;
class Settings;
struct Commands;

/**
 * Registers GET and POST /settings.
 * @param {AsyncWebServer&} server - server the routes are added to
 * @param {Commands&} commands - queue the settings changed flag is written to
 * @param {Settings&} settings - settings read and written by the routes
 * @returns {void}
 */
void registerSettingsRoutes(AsyncWebServer& server, Commands& commands, Settings& settings);
