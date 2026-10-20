#pragma once

#include "Animation.h"

/** Dark matrix with a softly blinking marker pixel in the top right corner. */
class Unread : public Animation {
public:
  const char* name() const override;
  void begin(Display& display) override;
  void tick(Display& display, uint32_t nowMs) override;
  void applyBrightness(Display& display, uint32_t fadeMs) override;

private:
  uint32_t startMs_ = 0;
  bool lit_ = false;
};
