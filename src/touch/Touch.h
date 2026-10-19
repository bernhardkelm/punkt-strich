#pragma once

#include <cstdint>

#include "touch/TouchBaseline.h"
#include "touch/TouchEvent.h"
#include "touch/TouchProximity.h"

/** Snapshot of the touch input, plain PODs so the http task can copy it while loop() writes. */
struct TouchStatus {
  uint32_t raw;
  uint32_t baseline;
  bool touched;
  TouchEvent lastEvent;
  /** millis() of the last TAP, DOUBLE_TAP or HOLD, 0 while none was reported */
  uint32_t lastEventMs;
  /** how often the freeze guard restarted the touch peripheral since boot */
  uint16_t restarts;
  /** hover level 0 to 3, 3 on contact, 0 while the hover levels are off */
  uint8_t proximity;
};

/**
 * Capacitive electrode on one touch pin, polled from loop() and reported as tap, tap confirmed,
 * double tap or hold. A press released between kHoldMs and kHoldMaxMs reports HOLD once, a press
 * released at or after kHoldMaxMs reports nothing because the reset fill was already running and
 * the gesture must leave the sessions alone. A shorter press reports TAP at once on its release
 * and starts a kDoubleTapMs window: a second press starting inside that window and ending as a
 * short press reports DOUBLE_TAP on its own release, otherwise TAP_CONFIRMED is reported once the
 * window closes with the pad released. A press passing kHoldMs drops the waiting confirmation,
 * its TAP already happened on the earlier release.
 * The pad on kLeftPin wakes the chip from the night sleep, its threshold is the awake trigger
 * delta. Every instance shares the one touch peripheral, a restart configures all begun pads.
 * The peripheral can come up dead after a portal restart and then reports one constant value, the
 * freeze guard restarts it and calibrates again once the reading stopped moving for kFrozenMs.
 * Below the trigger the same reading still says how close a finger is, TouchProximity turns it
 * into the hover levels 0 to 3 that proximity() reports, with contact always reading as the top
 * level. The levels only pause the baseline drift for kHoverHoldMs, after that an object left
 * near the pad is drifted into the baseline again and the levels fall back to 0 on their own.
 */
class Touch {
public:
  /** pin of the punkt coin, the left coin on strich and the only pad that wakes the chip */
  static constexpr uint8_t kLeftPin = 4;
  // a bare wire has little capacitance, 0x1000 charge cycles lift the raw counts out of the noise
  static constexpr uint16_t kMeasureCycles = 0x1000;
  static constexpr uint16_t kSleepCycles = 0xf;
  /** shortest press that reports HOLD on its release */
  static constexpr uint32_t kHoldMs = 800;
  /** a press released at or after this length is a reset attempt and reports no gesture */
  static constexpr uint32_t kHoldMaxMs = 2000;
  /** hover level reported while the pad is touched, the top level proximity() reports */
  static constexpr uint8_t kContactLevel = 3;

  /**
   * Binds the electrode to its pin, the peripheral is left alone until begin().
   * @param {uint8_t} pin - GPIO carrying a touch channel, the S3 channel number equals the pin
   * @param {const char*} name - log label after "punkt: ", "touch" keeps the punkt log lines
   */
  explicit Touch(uint8_t pin = kLeftPin, const char* name = "touch");

  /**
   * Applies the measurement cycles, warms the channel up and calibrates the baseline.
   * Blocks for about 220 ms, the first touchRead() also costs the channel init delay. After a
   * touch wake the finger may still rest on the pad during the calibration, the baseline then
   * starts too high and snapDown() corrects it once the finger lifts.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void begin(uint32_t nowMs);

  /**
   * Samples the electrode when the sample interval elapsed and recognises the gesture.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {TouchEvent} - TAP, DOUBLE_TAP, HOLD or TAP_CONFIRMED once per gesture, NONE
   * otherwise
   */
  TouchEvent tick(uint32_t nowMs);

  /**
   * Sets how far above the baseline a reading must rise to count as a touch, the release
   * threshold follows at kReleaseShare of it.
   * @param {uint8_t} percent - trigger as percent above the baseline, 5 to 250
   * @returns {void}
   */
  void setTriggerPercent(uint8_t percent);

  /**
   * Sets where the first hover level sits, the two levels above it follow from the trigger delta.
   * @param {uint8_t} percent - level 1 as percent of the trigger delta, 0 to 50, 0 is off
   * @returns {void}
   */
  void setHoverPercent(uint8_t percent);

  /**
   * Returns the raw delta above the baseline the sleep pad wakes at, the same margin the awake
   * trigger uses.
   * @returns {uint32_t} - threshold in raw counts above the baseline
   */
  uint32_t sleepThreshold() const;

  /**
   * Returns how close a finger is, the contact level while the pad is touched, the hover level
   * otherwise.
   * @returns {uint8_t} - hover level 0 to 3, 0 while the levels are off or the electrode is stuck
   */
  uint8_t proximity() const;

  /**
   * Returns whether a reported tap still waits for its double tap window.
   * @returns {bool} - true while a tap is pending its double tap window
   */
  bool tapPending() const;

  /**
   * Returns whether the debounced electrode is touched right now.
   * @returns {bool} - true from the sample the touch was debounced until the one it was released
   */
  bool touched() const;

  /**
   * Returns the pin the electrode was bound to.
   * @returns {uint8_t} - GPIO number
   */
  uint8_t pin() const;

  /**
   * Returns how long the electrode has been touched.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {uint32_t} - milliseconds since the debounced touch, 0 while released
   */
  uint32_t heldMs(uint32_t nowMs) const;

  /**
   * Enables the periodic raw reading log.
   * @param {bool} enabled - true prints a line every kDebugIntervalMs
   * @returns {void}
   */
  void setDebug(bool enabled);

  /**
   * Returns a copy of the current readings for the http task, which only reads.
   * @returns {TouchStatus} - raw, baseline, touched flag, last event, restart count and hover level
   */
  TouchStatus status() const;

private:
  static constexpr uint32_t kSampleMs = 20;
  static constexpr uint8_t kWarmupReads = 4;
  static constexpr uint8_t kBaselineSamples = 16;
  static constexpr uint32_t kCalibrationDelayMs = 10;
  /** release threshold as a share of the trigger, 25 percent trigger releases at 10 */
  static constexpr uint32_t kReleaseShare = 40;
  static constexpr uint8_t kDebounceSamples = 3;
  /** untouched reading at or below this percent of the baseline replaces the baseline at once */
  static constexpr uint32_t kSnapBelowPercent = 90;
  /** a second press starting within this window of the last release pairs into a double tap */
  static constexpr uint32_t kDoubleTapMs = 300;
  /** a touch this long is treated as a capacitance step, the baseline tracks again to recover */
  static constexpr uint32_t kStuckMs = 30000;
  static constexpr uint32_t kDriftDivisor = 64;
  static constexpr uint32_t kDriftIntervalMs = 200;
  /** longest baseline drift pause an active hover level can ask for */
  static constexpr uint32_t kHoverHoldMs = 5000;
  static constexpr uint32_t kDebugIntervalMs = 200;
  /**
   * a live electrode moves by tens of counts between samples, an untouched reading identical for
   * this long, about 500 samples at kSampleMs, means the peripheral stopped measuring
   */
  static constexpr uint32_t kFrozenMs = 10000;
  /** settle time the core waits after touch_pad_config() before the channel reads */
  static constexpr uint32_t kChannelSettleMs = 20;

  /**
   * Applies the measurement cycles, warms the channel up and averages kBaselineSamples readings
   * into the baseline, resetting the sample, drift, debug and freeze timers to nowMs.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void calibrate(uint32_t nowMs);

  /**
   * Averages kBaselineSamples readings into the baseline and records their max minus min as the
   * noise spread the hover levels stay above.
   * @returns {void}
   */
  void sampleBaseline();

  /**
   * Restarts the peripheral once an untouched raw reading stayed identical for kFrozenMs and
   * calibrates again, which also restarts the window so a dead pad cannot loop.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {bool} - true in the sample the peripheral was restarted
   */
  bool guardFrozen(uint32_t nowMs);

  /**
   * Tears the touch peripheral down and brings it back up the way the arduino core does on its
   * one time init, which it skips on every later call because of its static initialized flag,
   * then configures every pad an instance has begun on, not only its own.
   * @returns {void}
   */
  void restartSensor();

  /**
   * Counts samples that disagree with the touched flag and flips it once they are consistent.
   * @returns {bool} - true in the sample the flag flipped
   */
  bool debounce();

  /**
   * Replaces the baseline once untouched readings stay at or below kSnapBelowPercent of it for
   * kDebounceSamples, so a removed electrode does not leave an inflated baseline to the slow drift.
   * @returns {void}
   */
  void snapDown();

  /**
   * Returns the baseline a snap may fall to, the raw reading clamped so the level the snap came
   * from still counts as released against the new baseline.
   * @returns {uint32_t} - new reference level in raw counts
   */
  uint32_t snapTarget() const;

  /**
   * Moves the baseline towards the raw reading, frozen while a flip is pending and while touched
   * up to kStuckMs.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void trackDrift(uint32_t nowMs);

  /**
   * Returns whether an active hover level still pauses the baseline drift, timing the pause from
   * the sample the level left 0 so an object resting near the pad stops holding it after
   * kHoverHoldMs.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {bool} - true while the hover level pauses the drift
   */
  bool hoverHoldsDrift(uint32_t nowMs);

  /**
   * Returns whether the running touch passed kStuckMs, logging the first sample that does.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {bool} - true once the touch is treated as a stuck electrode
   */
  bool stuckTouch(uint32_t nowMs);

  /**
   * Prints the raw reading at the debug interval.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void logDebug(uint32_t nowMs);

  /**
   * Starts the press timing and logs the down edge.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {TouchEvent} - always NONE, every gesture is decided on the release
   */
  TouchEvent reportDown(uint32_t nowMs);

  /**
   * Logs the up edge and decides the gesture from the press length: HOLD between kHoldMs and
   * kHoldMaxMs, nothing at or above kHoldMaxMs, and below kHoldMs either DOUBLE_TAP pairing with
   * the waiting tap or TAP reported at once, which also starts its own double tap window.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {TouchEvent} - HOLD, DOUBLE_TAP, TAP or NONE for a reset attempt release
   */
  TouchEvent reportUp(uint32_t nowMs);

  /**
   * Confirms the waiting tap once its double tap window passed with the pad released, the tap
   * itself was already reported on its release.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {TouchEvent} - TAP_CONFIRMED or NONE
   */
  TouchEvent reportTapConfirmed(uint32_t nowMs);

  /**
   * Drops a waiting tap's confirmation once the running press passed kHoldMs, the tap itself was
   * already reported on the earlier release, only TAP_CONFIRMED is skipped, and its own release
   * can only be a hold or a reset attempt.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void dropTapOnHold(uint32_t nowMs);

  /**
   * Stores an event for the status snapshot.
   * @param {TouchEvent} event - event to store
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {TouchEvent} - the stored event
   */
  TouchEvent record(TouchEvent event, uint32_t nowMs);

  TouchBaseline baseline_;
  TouchProximity proximity_;
  const char* name_;
  uint8_t pin_;
  /** thresholds as percent of the baseline, replaced by setTriggerPercent() before any tick */
  uint32_t triggerPercent_ = 125;
  uint32_t releasePercent_ = 110;
  uint32_t raw_ = 0;
  /** max minus min of the readings the last calibration averaged */
  uint32_t spread_ = 0;
  uint32_t sampleMs_ = 0;
  uint32_t driftMs_ = 0;
  /** millis() the hover level last left 0 */
  uint32_t hoverMs_ = 0;
  uint32_t debugMs_ = 0;
  uint32_t pressedMs_ = 0;
  /** millis() of the release the waiting tap came from */
  uint32_t tapReleasedMs_ = 0;
  uint32_t lastEventMs_ = 0;
  /** raw reading the freeze window is running on */
  uint32_t frozenRaw_ = 0;
  /** millis() the raw reading last moved, last was touched, or the sensor last restarted */
  uint32_t frozenMs_ = 0;
  uint16_t restarts_ = 0;
  /** consecutive samples disagreeing with the touched flag */
  uint8_t pending_ = 0;
  /** consecutive untouched samples at or below kSnapBelowPercent of the baseline */
  uint8_t lowSamples_ = 0;
  /** level 1 as percent of the trigger delta, replaced by setHoverPercent() before any tick */
  uint8_t hoverPercent_ = 8;
  bool touched_ = false;
  /** the running press passed kHoldMs, the waiting tap confirmation was dropped */
  bool held_ = false;
  /** a reported tap waits for a second press to pair with, or for its confirmation */
  bool tapPending_ = false;
  /** stuck electrode already logged for the running touch */
  bool stuck_ = false;
  /** the hover level is above 0 and hoverMs_ holds the sample it left 0 */
  bool hovering_ = false;
  bool debug_ = false;
  TouchEvent lastEvent_ = TouchEvent::NONE;
};
