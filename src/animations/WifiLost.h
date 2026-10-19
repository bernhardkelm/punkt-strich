#pragma once

#include "Animation.h"

#include <cstdint>

/** Full Wi-Fi symbol blinking a second on, a second off, while the link is down. */
class WifiLost : public Animation {
public:
  const char* name() const override;
  void begin(Display& display) override;
  void tick(Display& display, uint32_t nowMs) override;

private:
  static constexpr uint32_t kOnMs = 1000;
  static constexpr uint32_t kOffMs = 1000;
  static constexpr uint32_t kPeriodMs = kOnMs + kOffMs;

  uint32_t startMs_ = 0;
  bool lit_ = false;

  /**
   * Draws the symbol or the dark matrix.
   * @param {Display&} display - matrix to draw on
   * @param {bool} lit - true shows the full symbol
   * @returns {void}
   */
  void draw(Display& display, bool lit) const;
};
