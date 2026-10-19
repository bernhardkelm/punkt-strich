#include "web/HttpServer.h"

#include "Commands.h"
#include "web/apiCommon.h"
#include "web/cityRoutes.h"
#include "web/percentRoutes.h"
#include "web/previewRoutes.h"
#include "web/settingsPage.h"
#include "web/settingsRoutes.h"
#include "web/stateRoutes.h"
#include "web/viewRoutes.h"

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

namespace {

constexpr uint16_t kPort = 80;
constexpr const char* kHtmlContentType = "text/html";
constexpr const char* kNotFoundBody = "{\"error\":\"not found\"}";
constexpr const char* kOkBody = "{\"ok\":true}";
constexpr uint32_t kListenCheckMs = 5000;

AsyncWebServer server(kPort);
Commands* commandQueue = nullptr;
uint32_t listenCheckedMs = 0;
bool started = false;
bool listening = false;

/**
 * Starts the listening socket and logs the outcome, AsyncServer::begin() itself fails silently.
 * @returns {bool} - true when the socket listens on kPort
 */
bool listen() {
  server.end();
  server.begin();

  if (server.state() != LISTEN) {
    Serial.printf("punkt: http failed to listen on port %u, retrying\n", kPort);
    return false;
  }

  Serial.printf("punkt: http listening on port %u\n", kPort);
  return true;
}

/**
 * Streams the settings page straight from flash instead of copying it into a String.
 * @param {AsyncWebServerRequest*} request - request to answer
 * @returns {void}
 */
void handleRoot(AsyncWebServerRequest* request) {
  logApi("GET", "/", "");
  request->send(200, kHtmlContentType, reinterpret_cast<const uint8_t*>(kSettingsPage),
                sizeof(kSettingsPage) - 1);
}

/**
 * Answers POST /reboot and asks loop() to restart once the client is gone, the flag is raised
 * from the disconnect callback so a restart cannot cut the response off.
 * @param {AsyncWebServerRequest*} request - request to answer
 * @returns {void}
 */
void handleReboot(AsyncWebServerRequest* request) {
  logApi("POST", "/reboot", "");
  request->onDisconnect([]() {
    commandQueue->reboot = true;
  });
  request->send(200, kJsonContentType, kOkBody);
}

/**
 * Answers POST /forget-wifi and asks loop() to erase the credentials and restart, the flag is
 * raised from the disconnect callback like the reboot one.
 * @param {AsyncWebServerRequest*} request - request to answer
 * @returns {void}
 */
void handleForgetWifi(AsyncWebServerRequest* request) {
  logApi("POST", "/forget-wifi", "");
  request->onDisconnect([]() {
    commandQueue->forgetWifi = true;
  });
  request->send(200, kJsonContentType, kOkBody);
}

/**
 * Answers everything unrouted, a POST to a JSON route lands here without the JSON content type.
 * @param {AsyncWebServerRequest*} request - request to answer
 * @returns {void}
 */
void handleNotFound(AsyncWebServerRequest* request) {
  const String& path = request->url();
  const bool jsonRoute = path == "/state" || path == "/settings" || path == "/context" ||
                         path == "/usage" || path == "/view" || path == "/preview";

  if (request->method() == HTTP_POST && jsonRoute) {
    sendApiError(request, "body must be JSON, send Content-Type: application/json");
    return;
  }

  logApi(request->methodToString(), path.c_str(), "not found");
  request->send(404, kJsonContentType, kNotFoundBody);
}

}

namespace HttpServer {

void begin(Commands& commands, Settings& settings, const StateMachine& state, const Touch& touch,
           const Touch& touchRight) {
  commandQueue = &commands;

  server.on(AsyncURIMatcher::exact("/"), HTTP_GET, handleRoot);
  registerStateRoutes(server, commands, settings, state, touch, touchRight);
  registerSettingsRoutes(server, commands, settings);
  registerPercentRoutes(server, commands);
  registerViewRoutes(server, commands, state);
  registerPreviewRoutes(server, commands, state);
  registerCityRoutes(server);
  server.on(AsyncURIMatcher::exact("/reboot"), HTTP_POST, handleReboot);
  server.on(AsyncURIMatcher::exact("/forget-wifi"), HTTP_POST, handleForgetWifi);
  server.onNotFound(handleNotFound);
  started = true;
  listenCheckedMs = millis();
  listening = listen();
}

void tick(uint32_t nowMs) {
  if (!started || (nowMs - listenCheckedMs) < kListenCheckMs) {
    return;
  }

  listenCheckedMs = nowMs;

  if (listening && server.state() == LISTEN) {
    return;
  }

  listening = listen();
}

}
