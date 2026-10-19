#include "touch/Touch.h"

#include <Arduino.h>
#include <driver/touch_sensor.h>

namespace {

/** bit per touch channel an instance has begun on, a restart of the shared peripheral needs all */
uint32_t begunPads = 0;

}

void Touch::begin(uint32_t nowMs) {
  const int8_t channel = digitalPinToTouchChannel(pin_);

  if (channel >= 0) {
    begunPads |= 1u << channel;
  }

  calibrate(nowMs);
}

void Touch::calibrate(uint32_t nowMs) {
  // cycles set before the warm-up reads, the baseline must be averaged at the final length
  touchSetCycles(kMeasureCycles, kSleepCycles);

  for (uint8_t i = 0; i < kWarmupReads; i++) {
    touchRead(pin_);
    delay(kCalibrationDelayMs);
  }

  sampleBaseline();
  raw_ = baseline_.value();
  proximity_.reset(raw_);
  sampleMs_ = nowMs;
  driftMs_ = nowMs;
  debugMs_ = nowMs;
  frozenMs_ = nowMs;
  frozenRaw_ = raw_;
  hoverMs_ = nowMs;
  hovering_ = false;
  Serial.printf("punkt: %s baseline %u spread %u\n", name_, baseline_.value(), spread_);
}

void Touch::sampleBaseline() {
  uint32_t sum = 0;
  uint32_t lowest = UINT32_MAX;
  uint32_t highest = 0;

  for (uint8_t i = 0; i < kBaselineSamples; i++) {
    const uint32_t sample = touchRead(pin_);
    sum += sample;

    if (sample < lowest) {
      lowest = sample;
    }

    if (sample > highest) {
      highest = sample;
    }

    delay(kCalibrationDelayMs);
  }

  baseline_.reset(sum / kBaselineSamples);
  spread_ = highest - lowest;
}

bool Touch::guardFrozen(uint32_t nowMs) {
  if (touched_ || raw_ != frozenRaw_) {
    frozenRaw_ = raw_;
    frozenMs_ = nowMs;
    return false;
  }

  if ((nowMs - frozenMs_) < kFrozenMs) {
    return false;
  }

  restartSensor();
  restarts_++;
  Serial.printf("punkt: %s frozen, sensor restarted\n", name_);
  calibrate(millis());
  return true;
}

void Touch::restartSensor() {
  touch_pad_fsm_stop();
  touch_pad_deinit();

  touch_pad_init();
  touchSetCycles(kMeasureCycles, kSleepCycles);
  touch_pad_set_voltage(TOUCH_PAD_HIGH_VOLTAGE_THRESHOLD, TOUCH_PAD_LOW_VOLTAGE_THRESHOLD,
                        TOUCH_PAD_ATTEN_VOLTAGE_THRESHOLD);
  touch_pad_set_idle_channel_connect(TOUCH_PAD_IDLE_CH_CONNECT_DEFAULT);

  touch_pad_denoise_t denoise = {};
  denoise.grade = TOUCH_PAD_DENOISE_BIT4;
  denoise.cap_level = TOUCH_PAD_DENOISE_CAP_L4;
  touch_pad_denoise_set_config(&denoise);
  touch_pad_denoise_enable();

  touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER);
  touch_pad_fsm_start();

  // touch_pad_deinit() clears the channel mask, every pad has to be configured again to be measured
  for (uint8_t pad = 0; pad < TOUCH_PAD_MAX; pad++) {
    if ((begunPads & (1u << pad)) != 0) {
      touch_pad_config(static_cast<touch_pad_t>(pad));
    }
  }

  touch_pad_intr_enable(TOUCH_PAD_INTR_MASK_ACTIVE | TOUCH_PAD_INTR_MASK_INACTIVE);
  delay(kChannelSettleMs);
}
