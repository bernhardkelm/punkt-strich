#include "power/deepSleep.h"

#include "Display.h"
#include "StatusLed.h"
#include "touch/Touch.h"

#include <Arduino.h>
#include <WiFi.h>
#include <driver/touch_sensor.h>
#include <esp_attr.h>
#include <esp_sleep.h>
#include <esp_system.h>

namespace {

constexpr uint32_t kSettleMs = 50;

/** sleep the board entered last, RTC memory keeps it through the deep sleep */
RTC_DATA_ATTR SleepKind sleptAs = SleepKind::NONE;
/** touch wake threshold the last sleep armed */
RTC_DATA_ATTR uint32_t sleptThreshold = 0;

/**
 * Takes every touch channel but the wake pad's out of the scan mask, so the right coin of strich
 * is not measured while asleep.
 * @param {int8_t} channel - touch channel of the wake pad
 * @returns {void}
 */
void scanOnly(int8_t channel) {
  const uint16_t others = static_cast<uint16_t>(TOUCH_PAD_BIT_MASK_ALL & ~(1u << channel));
  // the scan mask may only change while the FSM is stopped
  touch_pad_fsm_stop();
  touch_pad_clear_channel_mask(others);
  // esp_sleep_start() hands the sleep pad to the sleep FSM only while the FSM runs
  touch_pad_fsm_start();
}

/**
 * Arms the pad on pin as the one touch wake pad. Works on a calibrated peripheral and on a boot
 * that never began a Touch, where the wake enable initialises the peripheral itself.
 * @param {uint8_t} pin - GPIO of the wake pad
 * @param {uint32_t} threshold - raw delta above the sleep benchmark that wakes the chip
 * @returns {void}
 */
void armTouchWake(uint8_t pin, uint32_t threshold) {
  const int8_t channel = digitalPinToTouchChannel(pin);

  if (channel < 0) {
    return;
  }

  // set before the wake enable, an uninitialised peripheral starts with these cycle counts
  touchSetCycles(Touch::kMeasureCycles, Touch::kSleepCycles);
  touchSleepWakeUpEnable(pin, threshold);
  scanOnly(channel);
  // the sleep pad measures with its own cycle counts, the awake ones keep the threshold on scale
  touch_pad_sleep_channel_set_work_time(Touch::kSleepCycles, Touch::kMeasureCycles);
}

}

void logSleepWake() {
  const esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();

  if (cause != ESP_SLEEP_WAKEUP_TOUCHPAD && cause != ESP_SLEEP_WAKEUP_TIMER) {
    return;
  }

  const char* source = cause == ESP_SLEEP_WAKEUP_TOUCHPAD ? "touch" : "timer";
  const char* sleep = wokeFrom() == SleepKind::BATTERY ? "battery" : "night";
  Serial.printf("punkt: woke by %s from the %s sleep\n", source, sleep);
}

SleepKind wokeFrom() {
  // RTC memory is reloaded from the image on every reset but a deep sleep wake
  if (esp_reset_reason() != ESP_RST_DEEPSLEEP) {
    return SleepKind::NONE;
  }

  return sleptAs;
}

uint32_t storedWakeThreshold() {
  if (wokeFrom() == SleepKind::NONE) {
    return 0;
  }

  return sleptThreshold;
}

void darkenForSleep(Display& display) {
  display.standby();
  StatusLed::off();
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

void startDeepSleep(SleepKind kind, uint8_t pin, uint32_t threshold) {
  sleptAs = kind;
  sleptThreshold = threshold;
  armTouchWake(pin, threshold);
  Serial.flush();
  delay(kSettleMs);
  esp_deep_sleep_start();
}
