#include "power/VbusSense.h"

#include "board/boardConfig.h"
#include "power/batteryEstimate.h"

#include <Arduino.h>

namespace {

/** fast enough to rebase the coins long before a two-finger reset could arm */
constexpr uint32_t kSampleIntervalMs = 200;
constexpr uint8_t kSampleCount = 16;
constexpr uint32_t kDividerRatio = 2;
/** Pin level above which USB is present, battery VBUS reads at most about 1700 mV on the pin */
constexpr uint32_t kUsbThresholdMv = 2250;
/** Samples the smoothing averages over, a time constant of 3.2 s at kSampleIntervalMs */
constexpr int32_t kSmoothingSamples = 16;
/** VBUS the smoothed reading must move past the shown percent before the percent changes */
constexpr int kHoldMv = 4;
constexpr int kUnknownPercent = -1;

uint32_t lastSampleMs = 0;
bool sampled = false;
bool usb = false;
bool usbChanged = false;
int vbus = 0;
/** smoothed battery VBUS times kSmoothingSamples, seeded by the first battery sample */
int32_t smoothedScaled = 0;
int shownPercent = kUnknownPercent;

/**
 * Reads the sense pin kSampleCount times and returns the mean.
 * @returns {uint32_t} - pin voltage in millivolts
 */
uint32_t readPinMv() {
  uint32_t sum = 0;

  for (uint8_t i = 0; i < kSampleCount; i++) {
    sum += analogReadMilliVolts(Board::kVbusSensePin);
  }

  return sum / kSampleCount;
}

/**
 * Feeds one battery sample into the smoothed VBUS and moves the shown percent once the estimate
 * of the smoothed reading no longer covers it within kHoldMv. The first sample after USB or boot
 * seeds the smoothing.
 * @param {int} mv - VBUS sample in millivolts
 * @returns {void}
 */
void smoothBattery(int mv) {
  if (shownPercent == kUnknownPercent) {
    smoothedScaled = mv * kSmoothingSamples;
  } else {
    smoothedScaled += mv - smoothedScaled / kSmoothingSamples;
  }

  const int smoothed = static_cast<int>(smoothedScaled / kSmoothingSamples);
  const int low = batteryPercent(smoothed - kHoldMv);
  const int high = batteryPercent(smoothed + kHoldMv);

  if (shownPercent >= low && shownPercent <= high) {
    return;
  }

  shownPercent = batteryPercent(smoothed);
}

}

namespace VbusSense {

void begin() {
  if (!Board::kHasVbusSense) {
    return;
  }

  analogSetPinAttenuation(Board::kVbusSensePin, ADC_11db);
}

void tick(uint32_t nowMs) {
  if (!Board::kHasVbusSense) {
    return;
  }

  if (sampled && nowMs - lastSampleMs < kSampleIntervalMs) {
    return;
  }

  const bool first = !sampled;
  sampled = true;
  lastSampleMs = nowMs;
  const uint32_t pinMv = readPinMv();
  const bool present = pinMv > kUsbThresholdMv;
  vbus = static_cast<int>(pinMv * kDividerRatio);
  usbChanged = usbChanged || (!first && present != usb);
  usb = present;

  // on USB the pin reads the charger, not the cell
  if (usb) {
    shownPercent = kUnknownPercent;
    return;
  }

  smoothBattery(vbus);
}

bool consumeUsbChange() {
  const bool changed = usbChanged;
  usbChanged = false;
  return changed;
}

bool usbPresent() {
  return usb;
}

int vbusMv() {
  return vbus;
}

int percent() {
  return shownPercent;
}

}
