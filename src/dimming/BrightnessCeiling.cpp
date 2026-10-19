#include "dimming/BrightnessCeiling.h"

#include "Display.h"
#include "dimming/AutoDim.h"
#include "dimming/RestDim.h"
#include "dimming/restCeiling.h"
#include "settings/Settings.h"
#include "state/State.h"

#include <Arduino.h>

BrightnessCeiling::BrightnessCeiling(Settings& settings, StateMachine& state, AutoDim& autoDim,
                                     RestDim& restDim)
    : settings_(settings),
      state_(state),
      autoDim_(autoDim),
      restDim_(restDim),
      applied_(Settings::kDefaultBrightness) {
}

void BrightnessCeiling::refresh() {
  autoDim_.refresh(millis(), settings_);
  restDim_.tick(millis(), settings_, state_);
  apply(Display::kFadeMs);
}

void BrightnessCeiling::tick(uint32_t nowMs) {
  uint8_t dimLevel = 0;
  const bool sunChanged = autoDim_.tick(nowMs, settings_, dimLevel);
  const bool restChanged = restDim_.tick(nowMs, settings_, state_);

  if (sunChanged || restChanged) {
    const uint32_t fadeMs =
        (restChanged && restDim_.resting()) ? RestDim::kRampMs : Display::kFadeMs;
    apply(fadeMs);
  }
}

uint8_t BrightnessCeiling::level() const {
  return applied_;
}

void BrightnessCeiling::apply(uint32_t fadeMs) {
  const uint8_t level =
      restCeiling(autoDim_.level(), settings_.nightBrightness(), restDim_.resting());
  applied_ = level;
  state_.setBrightness(level, fadeMs);
  state_.setDimmed(restDim_.resting());
}
