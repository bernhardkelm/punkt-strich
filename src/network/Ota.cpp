#include "network/Ota.h"

#include <ArduinoOTA.h>

namespace {

Ota::StartCallback startCallback = nullptr;
Ota::ProgressCallback progressCallback = nullptr;
Ota::ErrorCallback errorCallback = nullptr;

/**
 * Returns a readable name for an ArduinoOTA error code.
 * @param {ota_error_t} error - code reported by the library
 * @returns {const char*} - short error name
 */
const char* errorName(ota_error_t error) {
  switch (error) {
    case OTA_AUTH_ERROR:
      return "auth";
    case OTA_BEGIN_ERROR:
      return "begin";
    case OTA_CONNECT_ERROR:
      return "connect";
    case OTA_RECEIVE_ERROR:
      return "receive";
    case OTA_END_ERROR:
      return "end";
  }

  return "unknown";
}

}

namespace Ota {

void begin(const char* name, StartCallback onStart, ProgressCallback onProgress,
           ErrorCallback onError) {
  startCallback = onStart;
  progressCallback = onProgress;
  errorCallback = onError;

  ArduinoOTA.setHostname(name);

  ArduinoOTA.onStart([]() {
    Serial.println("punkt: ota start");

    if (startCallback != nullptr) {
      startCallback();
    }
  });

  ArduinoOTA.onProgress([](unsigned int done, unsigned int total) {
    if (progressCallback != nullptr) {
      progressCallback(done, total);
    }
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("punkt: ota done");
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("punkt: ota error %s\n", errorName(error));

    if (errorCallback != nullptr) {
      errorCallback();
    }
  });

  ArduinoOTA.begin();
}

void handle() {
  ArduinoOTA.handle();
}

}
