#pragma once

#include "Display.h"

#include <cstdint>

/** Base class for a non-blocking matrix animation, tick() must never delay(). */
class Animation {
public:
  virtual ~Animation() = default;

  /** Returns the animation name used in the switch log. */
  virtual const char* name() const = 0;

  /**
   * Resets the animation state and draws its first frame.
   * @param {Display&} display - cleared matrix at the default brightness
   * @returns {void}
   */
  virtual void begin(Display& display) = 0;

  /**
   * Advances the animation, called from loop() as often as possible.
   * @param {Display&} display - matrix to draw on
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  virtual void tick(Display& display, uint32_t nowMs) = 0;

  /**
   * Returns whether the animation has run to completion and the state may advance.
   * @returns {bool} - true once a one-shot animation is done
   */
  virtual bool finished() const {
    return false;
  }

  /**
   * Sets the brightness ceiling animations scale against, may change while running.
   * @param {uint8_t} level - 0 (dimmest) to 15 (brightest)
   * @returns {void}
   */
  void setMaxBrightness(uint8_t level) {
    maxBrightness_ = level;
  }

  /**
   * Ramps the display to the level the animation runs at under the current ceiling, called when
   * the ceiling changed while it runs. Animations that run below the ceiling override it.
   * @param {Display&} display - matrix to apply the level to
   * @param {uint32_t} fadeMs - ramp duration, 0 applies immediately
   * @returns {void}
   */
  virtual void applyBrightness(Display& display, uint32_t fadeMs) {
    display.setBrightness(maxBrightness_, fadeMs);
  }

  /**
   * Sets the context window fill the calm animations show as a gauge on the bottom row.
   * @param {uint8_t} percent - 0 hides the gauge, 1 to 100 lights round(percent * 8 / 100) pixels
   * @returns {void}
   */
  void setContextPercent(uint8_t percent) {
    contextPercent_ = percent > 100 ? 100 : percent;
  }

protected:
  /**
   * Returns the current brightness ceiling.
   * @returns {uint8_t} - 0 to 15
   */
  uint8_t maxBrightness() const {
    return maxBrightness_;
  }

  /**
   * Returns the context window fill for the gauge.
   * @returns {uint8_t} - 0 to 100, 0 when no gauge is shown
   */
  uint8_t contextPercent() const {
    return contextPercent_;
  }

private:
  uint8_t maxBrightness_ = 8;
  uint8_t contextPercent_ = 0;
};
