#include "web/cityRoutes.h"

#include "settings/cities.h"
#include "web/apiCommon.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>

namespace {

/**
 * Answers with the city table, one object per entry in table order.
 * @param {AsyncWebServerRequest*} request - request to answer
 * @returns {void}
 */
void handleGet(AsyncWebServerRequest* request) {
  JsonDocument doc;
  JsonArray list = doc.to<JsonArray>();

  for (size_t i = 0; i < cityCount(); i++) {
    const City& city = cityAt(i);
    JsonObject entry = list.add<JsonObject>();
    entry["name"] = city.name;
    entry["label"] = city.label;
    entry["region"] = city.region;
    entry["latitude"] = city.latitude;
    entry["longitude"] = city.longitude;
  }

  sendApiJson(request, doc, "");
}

}

void registerCityRoutes(AsyncWebServer& server) {
  server.on(AsyncURIMatcher::exact("/cities"), HTTP_GET, handleGet);
}
