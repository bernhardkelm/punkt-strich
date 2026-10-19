#pragma once

#include "Animation.h"
#include "clockFaces/LowBatteryDot.h"
#include "settings/Settings.h"

#include <cstdint>

/** Full-width 24-hour clock across the 16x8 canvas, digit layout chosen by the clock face. */
class WideClock : public Animation {
public:
  /**
   * Creates the clock face.
   * @param {const Settings&} settings - source of the selected face, must outlive the animation
   */
  explicit WideClock(const Settings& settings);

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

  /**
   * Sets whether the clock carries the blinking low-battery dot, which restarts its rhythm when it
   * turns on. The next tick redraws the frame when the dot changed.
   * @param {bool} on - true while the battery runs low
   * @returns {void}
   */
  void setLowBattery(bool on);

private:
  static constexpr int8_t kMarkerX = 8;
  static constexpr int8_t kMarkerY = 0;
  static constexpr uint32_t kPollMs = 1000;
  static constexpr uint32_t kBlinkPeriodMs = 1000;
  static constexpr uint32_t kBlinkOnMs = 500;
  static constexpr int8_t kWaitX = 7;
  static constexpr int8_t kWaitY = 3;
  static constexpr int16_t kNoMinute = -1;
  static constexpr int16_t kNoFace = -1;

  const Settings& settings_;
  uint32_t startMs_ = 0;
  uint32_t lastPollMs_ = 0;
  int16_t shownMinute_ = kNoMinute;
  int16_t shownFace_ = kNoFace;
  bool blinkLit_ = true;
  bool synced_ = false;
  bool marker_ = false;
  bool markerLit_ = false;
  uint32_t markerStartMs_ = 0;
  LowBatteryDot dot_;

  /**
   * Draws the single centre pixel shown while no time is available, and the low-battery dot while
   * it is active.
   * @param {Display&} display - canvas to draw on
   * @param {bool} lit - pixel state
   * @returns {void}
   */
  void drawWaiting(Display& display, bool lit) const;

  /**
   * Draws the low-battery dot over the canvas while it is active, leaves the pixel alone otherwise.
   * @param {Display&} display - canvas to draw on
   * @returns {void}
   */
  void drawDot(Display& display) const;

  /**
   * Draws the four digits of a time through drawClockFrame(), both pairs at the face's height, the
   * unread marker pixel while markerLit_ is set and the low-battery dot while it is active.
   * @param {Display&} display - canvas to draw on
   * @param {uint8_t} rows - digit height of the face, from clockFaceRows()
   * @param {uint8_t} hour - 0 to 23, shown with a leading zero
   * @param {uint8_t} minute - 0 to 59, shown with a leading zero
   * @returns {void}
   */
  void drawTime(Display& display, uint8_t rows, uint8_t hour, uint8_t minute);

  /**
   * Redraws the digits when the shown minute, the face or the marker pixel changed, leaves the
   * frame alone otherwise.
   * @param {Display&} display - canvas to draw on
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
