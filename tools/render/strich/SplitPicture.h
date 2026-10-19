#pragma once

#include "Animation.h"
#include "clockFaces/SideClock.h"
#include "settings/Settings.h"

#include <cstdint>
#include <memory>

/**
 * Split picture of the strich firmware: the side clock in the left half and a Claude picture in
 * the viewport beside it, begun and ticked the way the state machine runs a split picture.
 */
class SplitPicture : public Animation {
public:
  /**
   * Creates the split picture.
   * @param {const Settings&} settings - source of the split clock style, must outlive the picture
   * @param {std::unique_ptr<Animation>} picture - Claude picture shown in the viewport
   */
  SplitPicture(const Settings& settings, std::unique_ptr<Animation> picture);

  const char* name() const override;
  void begin(Display& display) override;
  void tick(Display& display, uint32_t nowMs) override;

private:
  SideClock sideClock_;
  std::unique_ptr<Animation> picture_;
};
