#include "state/State.h"

#include "power/VbusSense.h"
#include "state/lowBattery.h"

#include <Arduino.h>

bool StateMachine::wipesInto(const Picture& next) const {
  if (!kHasBatteryView || display_.revealing()) {
    return false;
  }

  const bool fromBattery = animation_ == &batteryFace_;
  const bool toBattery = &next.animation == &batteryFace_;

  if (fromBattery == toBattery) {
    return false;
  }

  const Animation& other = fromBattery ? next.animation : *animation_;
  const bool otherSplit = fromBattery ? next.split : split_;
  return otherSplit || &other == &wideClock_;
}

void StateMachine::applyBatteryTimeout(uint32_t nowMs) {
  if (!kHasBatteryView || view_ != View::BATTERY) {
    return;
  }

  const uint32_t behind = lastActivityMs_ - nowMs;

  // a nowMs sampled before the last activity would read as an elapsed eternity after the wrap
  if (behind != 0 && behind < kSampleLagMaxMs) {
    return;
  }

  if (nowMs - lastActivityMs_ < kBatteryViewMs) {
    return;
  }

  Serial.println("punkt: view clock (battery timeout)");
  changeView(View::CLOCK, nowMs);
}

void StateMachine::applyLowBattery() {
  if (!Board::kHasVbusSense) {
    return;
  }

  lowBattery_ = lowBatteryAfter(lowBattery_, VbusSense::usbPresent(), VbusSense::percent());
  wideClock_.setLowBattery(lowBattery_);
  sideClock_.setLowBattery(lowBattery_);
}
