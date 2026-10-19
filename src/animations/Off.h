#pragma once

#include "Animation.h"

/** Dark matrix with a dim heartbeat pixel in the bottom right corner. */
class Off : public Animation {
public:
  const char* name() const override;
  void begin(Display& display) override;
  void tick(Display& display, uint32_t nowMs) override;

private:
  static constexpr uint32_t kPeriodMs = 10000;
  static constexpr uint32_t kBlinkMs = 200;
  static constexpr uint8_t kBlinkBrightness = 1;

  uint32_t startMs_ = 0;
  bool lit_ = false;
};
