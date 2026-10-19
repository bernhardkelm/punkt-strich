#pragma once

#include <cstdint>

/** Debounced BOOT button on GPIO0, polled from loop() and reported once per release. */
class Button {
public:
  /**
   * Configures the pin and starts the hold timing, the button pulls it low against the internal
   * pull-up.
   * @returns {void}
   */
  void begin();

  /**
   * Samples the pin and debounces it.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {bool} - true in the iteration the button was released
   */
  bool tick(uint32_t nowMs);

  /**
   * Returns how long the button has been held down.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {uint32_t} - milliseconds since the debounced press, 0 while released
   */
  uint32_t heldMs(uint32_t nowMs) const;

  /**
   * Returns how long the press that tick() just reported lasted.
   * @returns {uint32_t} - milliseconds between the debounced press and release
   */
  uint32_t lastHoldMs() const;

private:
  static constexpr uint8_t kPin = 0;
  static constexpr uint32_t kDebounceMs = 50;

  bool raw_ = false;
  bool pressed_ = false;
  uint32_t changedMs_ = 0;
  uint32_t pressedMs_ = 0;
  uint32_t lastHoldMs_ = 0;
};
