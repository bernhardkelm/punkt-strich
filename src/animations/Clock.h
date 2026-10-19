#pragma once

#include "Animation.h"

/** Digital clock, hours over minutes as 3x4 digits, a blinking centre pixel while the time is not synced. */
class Clock : public Animation {
public:
  const char* name() const override;
  void begin(Display& display) override;
  void tick(Display& display, uint32_t nowMs) override;

  /**
   * Sets whether the clock carries the blinking unread marker pixel, which restarts its rhythm
   * when it turns on. The next tick redraws the frame when the pixel changed.
   * @param {bool} on - true while the displayed session is unread
   * @returns {void}
   */
  void setMarker(bool on);

private:
  static constexpr int8_t kMarkerX = 7;
  static constexpr int8_t kMarkerY = 0;
  static constexpr uint32_t kPollMs = 1000;
  static constexpr uint32_t kBlinkPeriodMs = 1000;
  static constexpr uint32_t kBlinkOnMs = 500;
  static constexpr int8_t kWaitX = 3;
  static constexpr int8_t kWaitY = 3;
  static constexpr int16_t kNoMinute = -1;
  static constexpr int8_t kHourY = 0;
  static constexpr int8_t kMinuteY = 4;
  static constexpr int8_t kHourTensX = 0;
  static constexpr int8_t kHourOnesX = 4;
  // the minute line sits one column right of the hour line so the two do not read as one block
  static constexpr int8_t kMinuteTensX = 1;
  static constexpr int8_t kMinuteOnesX = 5;
  static constexpr uint8_t kTens = 10;

  uint32_t startMs_ = 0;
  uint32_t lastPollMs_ = 0;
  int16_t shownMinute_ = kNoMinute;
  bool blinkLit_ = true;
  bool synced_ = false;
  bool marker_ = false;
  bool markerLit_ = false;
  uint32_t markerStartMs_ = 0;

  /**
   * Draws the single centre pixel shown while no time is available.
   * @param {Display&} display - matrix to draw on
   * @param {bool} lit - pixel state
   * @returns {void}
   */
  static void drawWaiting(Display& display, bool lit);

  /**
   * Draws the four digits of a time, hours on the upper line and minutes on the lower one, and the
   * unread marker pixel while markerLit_ is set.
   * @param {Display&} display - matrix to draw on
   * @param {uint8_t} hour - 0 to 23, shown with a leading zero
   * @param {uint8_t} minute - 0 to 59, shown with a leading zero
   * @returns {void}
   */
  void drawTime(Display& display, uint8_t hour, uint8_t minute);

  /**
   * Redraws the digits when the displayed minute or the marker pixel changed, leaves the frame
   * alone otherwise.
   * @param {Display&} display - matrix to draw on
   * @returns {bool} - false while the time is not synced yet
   */
  bool drawTimeIfChanged(Display& display);

  /**
   * Returns whether the marker pixel is lit at a moment, false while the marker is off.
   * @param {uint32_t} nowMs - millis() to evaluate
   * @returns {bool} - true while the marker is on and in the lit part of its rhythm
   */
  bool markerLitAt(uint32_t nowMs) const;
};
