#pragma once

#include "settings/Settings.h"

class BrightnessCeiling;
class FingerTracker;
class StateMachine;
class Touch;
class TouchOverlay;

/**
 * Hands the stored settings to the modules loop() runs, remembering the timezone and the power
 * profile it applied so an unrelated change does not touch them again.
 */
class SettingsApplier {
public:
  /**
   * Binds the applier to the settings and every module they reach, nothing is applied yet.
   * @param {Settings&} settings - settings read on every apply
   * @param {StateMachine&} state - state machine that re-selects its working animation
   * @param {BrightnessCeiling&} ceiling - brightness ceiling re-evaluated on every apply
   * @param {Touch&} touch - punkt's only coin, the left coin on strich
   * @param {Touch&} rightTouch - strich's right coin, never begun on punkt
   * @param {FingerTracker&} fingers - gesture source of strich's two coins
   * @param {TouchOverlay&} overlay - overlay the hover setting enables
   */
  SettingsApplier(Settings& settings, StateMachine& state, BrightnessCeiling& ceiling, Touch& touch,
                  Touch& rightTouch, FingerTracker& fingers, TouchOverlay& overlay);

  /**
   * Hands the stored timezone to the clock unconditionally and remembers it.
   * @returns {void}
   */
  void beginTimezone();

  /**
   * Applies the stored power profile unconditionally and remembers it.
   * @returns {void}
   */
  void beginPowerSave();

  /**
   * Hands the touch settings to one coin, both coins of strich share them.
   * @param {Touch&} coin - electrode the settings apply to
   * @returns {void}
   */
  void applyTouch(Touch& coin);

  /**
   * Hands the touch trigger and the debug flag to the finger tracker of strich.
   * @returns {void}
   */
  void applyFingers();

  /**
   * Re-reads the settings loop() acts on: the brightness ceiling goes through the automatic
   * dimming and the resting cap first, the power profile is only touched when the flag changed so
   * the CPU clock is not reset on every brightness change.
   * @returns {void}
   */
  void apply();

private:
  /**
   * Hands the timezone to the clock, only when the setting actually changed so a brightness or
   * animation change does not log a timezone line.
   * @returns {void}
   */
  void applyTimezone();

  Settings& settings_;
  StateMachine& state_;
  BrightnessCeiling& ceiling_;
  Touch& touch_;
  Touch& rightTouch_;
  FingerTracker& fingers_;
  TouchOverlay& overlay_;
  bool powerSaveApplied_ = Settings::kDefaultPowerSave;
  char timezoneApplied_[Settings::kTimezoneMax + 1] = {};
};
