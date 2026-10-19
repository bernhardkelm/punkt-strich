#include "network/TimeSync.h"

#include <Arduino.h>
#include <cstdlib>

namespace {

constexpr const char* kServer1 = "pool.ntp.org";
constexpr const char* kServer2 = "time.nist.gov";
constexpr time_t kSanityEpoch = 1700000000;
constexpr uint32_t kPollMs = 500;

bool started = false;
bool logged = false;
uint32_t lastPollMs = 0;

}

namespace TimeSync {

void begin(const char* tz) {
  if (started) {
    return;
  }

  started = true;
  configTzTime(tz, kServer1, kServer2);
  Serial.printf("punkt: time sync started tz=%s\n", tz);
}

void setTimezone(const char* tz) {
  setenv("TZ", tz, 1);
  tzset();
  Serial.printf("punkt: timezone %s\n", tz);
}

bool synced() {
  return time(nullptr) > kSanityEpoch;
}

bool localTime(tm& out) {
  if (!synced()) {
    return false;
  }

  const time_t now = time(nullptr);
  localtime_r(&now, &out);
  return true;
}

void tick(uint32_t nowMs) {
  if (logged || !started) {
    return;
  }

  if (nowMs - lastPollMs < kPollMs) {
    return;
  }

  lastPollMs = nowMs;

  tm local{};

  if (!localTime(local)) {
    return;
  }

  logged = true;
  Serial.printf("punkt: time synced %02d:%02d:%02d\n", local.tm_hour, local.tm_min, local.tm_sec);
}

}
