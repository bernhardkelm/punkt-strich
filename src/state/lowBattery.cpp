#include "state/lowBattery.h"

namespace {

/** Charge below which the battery turns low */
constexpr int kLowBelowPercent = 15;
/** Charge from which a low battery clears again */
constexpr int kClearFromPercent = 18;

}

bool lowBatteryAfter(bool wasLow, bool usb, int percent) {
  if (usb) {
    return false;
  }

  if (percent < 0) {
    return wasLow;
  }

  if (percent < kLowBelowPercent) {
    return true;
  }

  if (percent >= kClearFromPercent) {
    return false;
  }

  return wasLow;
}
