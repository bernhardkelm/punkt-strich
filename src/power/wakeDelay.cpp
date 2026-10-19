#include "power/wakeDelay.h"

namespace {

constexpr uint32_t kSecondsPerMinute = 60;
constexpr uint32_t kSecondsPerHour = 3600;
constexpr uint32_t kSecondsPerDay = 86400;

}

uint32_t secondsUntilWake(const tm& local, uint16_t wakeAt) {
  const uint32_t nowSec = static_cast<uint32_t>(local.tm_hour) * kSecondsPerHour +
                          static_cast<uint32_t>(local.tm_min) * kSecondsPerMinute +
                          static_cast<uint32_t>(local.tm_sec);
  const uint32_t wakeSec = static_cast<uint32_t>(wakeAt) * kSecondsPerMinute;
  const uint32_t delay = (wakeSec + kSecondsPerDay - nowSec % kSecondsPerDay) % kSecondsPerDay;
  return delay == 0 ? kSecondsPerDay : delay;
}
