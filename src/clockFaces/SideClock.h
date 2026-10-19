#pragma once

#include "Animation.h"
#include "clockFaces/LowBatteryDot.h"
#include "settings/Settings.h"

#include <cstdint>

/**
 * Split clock in the left half beside a split picture, drawn into the viewport like the punkt
 * clock, so the owner runs it at viewport 0. Shows splitClockFrame() in the style
 * Settings::splitClock() selects and a blinking centre pixel while the time is not synced. A new
 * minute or style redraws at once, except that a running SQUARES clock rolls to the next minute
 * through squaresRollStep(), one step per kRollStepMs. Carries the low-battery dot.
 */
class SideClock : public Animation {
public:
  /**
   * Creates the split clock.
   * @param {const Settings&} settings - source of the selected style, must outlive the animation
   */
  explicit SideClock(const Settings& settings);

  const char* name() const override;
  void begin(Display& display) override;
  void tick(Display& display, uint32_t nowMs) override;

  /**
   * Sets whether the clock carries the blinking low-battery dot, which restarts its rhythm when it
   * turns on. The next tick redraws the frame when the dot changed.
   * @param {bool} on - true while the battery runs low
   * @returns {void}
   */
  void setLowBattery(bool on);

private:
  static constexpr uint32_t kPollMs = 1000;
  static constexpr uint32_t kBlinkPeriodMs = 1000;
  static constexpr uint32_t kBlinkOnMs = 500;
  /** Wait between two pixels the squares drain on a tens or an hour rollover */
  static constexpr uint32_t kRollStepMs = 35;
  static constexpr int8_t kWaitX = 3;
  static constexpr int8_t kWaitY = 3;
  static constexpr uint16_t kMinutesPerHour = 60;

  const Settings& settings_;
  LowBatteryDot dot_;
  uint32_t startMs_ = 0;
  uint32_t lastPollMs_ = 0;
  /** millis() the next roll step is due at */
  uint32_t rollDueMs_ = 0;
  /** minute of the day the picture shows, behind targetMinute_ while the squares roll */
  uint16_t shownMinute_ = 0;
  /** minute of the day the clock read last */
  uint16_t targetMinute_ = 0;
  SplitClock style_ = Settings::kDefaultSplitClock;
  bool blinkLit_ = true;
  bool synced_ = false;

  /**
   * Reads the local time and the style, and moves the picture towards them: a running SQUARES
   * clock takes its first roll step, the first time and a new style show at once.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {bool} - true when the picture changed, false while unsynced or unchanged
   */
  bool pollTime(uint32_t nowMs);

  /**
   * Takes the next roll step once it is due.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {bool} - true when the picture changed
   */
  bool advanceRoll(uint32_t nowMs);

  /**
   * Replaces the viewport with the clock picture, or the waiting pixel while unsynced, and the
   * low-battery dot over it while the dot is active, then pushes the frame.
   * @param {Display&} display - viewport to draw on
   * @returns {void}
   */
  void draw(Display& display) const;
};
