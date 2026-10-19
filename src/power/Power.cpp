#include "power/Power.h"

#include <Arduino.h>
#include <WiFi.h>
#include <cmath>

namespace {

constexpr uint32_t kCpuFreqPowerSaveMhz = 80;
constexpr uint32_t kCpuFreqFullMhz = 240;
constexpr uint32_t kTempIntervalMs = 60000;
constexpr uint32_t kFirstReadDelayMs = 5000;

float lastTempC = NAN;
uint32_t lastTempMs = 0;
bool firstReadDone = false;

/**
 * Reads the chip temperature, stores it as the latest sample and logs it.
 * @param {uint32_t} nowMs - millis() at the time of the call
 * @returns {void}
 */
void sampleTemperature(uint32_t nowMs) {
  lastTempMs = nowMs;
  lastTempC = temperatureRead();
  Serial.printf("punkt: temp %.1fC cpu=%u\n", lastTempC, getCpuFrequencyMhz());
}

}

namespace Power {

const char* sleepLabel(wifi_ps_type_t mode) {
  switch (mode) {
    case WIFI_PS_NONE:
      return "none";
    case WIFI_PS_MIN_MODEM:
      return "min-modem";
    case WIFI_PS_MAX_MODEM:
      return "max-modem";
  }

  return "unknown";
}

void apply(bool powerSave) {
  setCpuFrequencyMhz(powerSave ? kCpuFreqPowerSaveMhz : kCpuFreqFullMhz);
  // min modem sleep wakes for every beacon, max modem sleep drops unicast frames on a weak link
  WiFi.setSleep(powerSave ? WIFI_PS_MIN_MODEM : WIFI_PS_NONE);

  const uint32_t cpuMhz = getCpuFrequencyMhz();
  const char* wifiLabel = sleepLabel(WiFi.getSleep());

  if (powerSave) {
    Serial.printf("punkt: power save on cpu=%u wifi=%s\n", cpuMhz, wifiLabel);
  } else {
    Serial.printf("punkt: power save off cpu=%u wifi=%s\n", cpuMhz, wifiLabel);
  }
}

void tick(uint32_t nowMs) {
  if (!firstReadDone) {
    if (nowMs < kFirstReadDelayMs) {
      return;
    }

    firstReadDone = true;
    sampleTemperature(nowMs);
    return;
  }

  if (nowMs - lastTempMs < kTempIntervalMs) {
    return;
  }

  sampleTemperature(nowMs);
}

float temperatureC() {
  return lastTempC;
}

}
