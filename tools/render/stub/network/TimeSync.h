#pragma once

#include <cstdint>
#include <ctime>

/** Host stub of the SNTP wrapper, hands the clock animation one fixed local time. */
namespace TimeSync {

/** Hour the rendered clock picture shows */
constexpr int kRenderHour = 14;
/** Minute the rendered clock picture shows */
constexpr int kRenderMinute = 7;

/**
 * Fills the broken-down local time with the fixed render time.
 * @param {tm&} out - receives the local time
 * @returns {bool} - always true, the host counts as synced from the first frame
 */
inline bool localTime(tm& out) {
  out = tm{};
  out.tm_hour = kRenderHour;
  out.tm_min = kRenderMinute;
  return true;
}

}
