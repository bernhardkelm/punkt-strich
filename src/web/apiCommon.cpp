#include "web/apiCommon.h"

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

void logApi(const char* method, const char* path, const char* detail) {
  if (detail == nullptr || detail[0] == '\0') {
    Serial.printf("punkt: api %s %s\n", method, path);
    return;
  }

  Serial.printf("punkt: api %s %s %s\n", method, path, detail);
}

void sendApiError(AsyncWebServerRequest* request, const char* message, uint16_t status) {
  logApi(request->methodToString(), request->url().c_str(), message);

  String body("{\"error\":\"");
  body += message;
  body += "\"}";
  request->send(status, kJsonContentType, body);
}

void sendApiJson(AsyncWebServerRequest* request, const JsonDocument& doc, const char* detail) {
  String body;
  serializeJson(doc, body);

  const char* logged = detail == nullptr ? body.c_str() : detail;
  logApi(request->methodToString(), request->url().c_str(), logged);
  request->send(200, kJsonContentType, body);
}
