#pragma once

#include "touch/ChordHold.h"
#include "touch/Coin.h"
#include "touch/FingerEvent.h"
#include "touch/FingerEventQueue.h"
#include "touch/FingerPress.h"
#include "touch/FingerSensor.h"
#include "touch/GestureLog.h"
#include "touch/GestureSettle.h"
#include "touch/SwipeDisc.h"
#include "touch/TapWindow.h"

#include <cstdint>

/**
 * Tracks one finger over the two coins of strich and reports its gestures. Both coins are sampled
 * every 10 ms into a filtered finger column (FingerSensor). A press starts once the stronger coin
 * holds kActiveShare for kNearSamples and ends once it stayed below for kGoneSamples. A press
 * turns into a swipe by the rules of FingerPress and reports SWIPE_BEGIN, then SWIPE_END on its
 * release. A press released as a tap reports TAP, or DOUBLE_TAP when its press started inside the
 * double tap window of an earlier tap on the same coin, and TAP_CONFIRMED follows once that
 * window passed without a press on the coin. A single-coin contact released after at least
 * 800 ms reports HOLD with its length, and drops a waiting tap confirmation once it passes 800 ms.
 * A two-finger hold (ChordHold) marks its press, which then reports nothing on its release, and
 * ends a running swipe with an incomplete SWIPE_END. A swipe, a hold or a chord release is
 * followed by a GestureSettle before the next press can start, a tap starts none.
 */
class FingerTracker {
public:
  /**
   * Binds the tracker to the coin pins, nothing is read until begin().
   * @param {uint8_t} leftPin - touch GPIO of the left coin, above columns 0 to 7
   * @param {uint8_t} rightPin - touch GPIO of the right coin, above columns 8 to 15
   */
  FingerTracker(uint8_t leftPin, uint8_t rightPin);

  /**
   * Calibrates both baselines with the finger away and resets the gesture state. Blocks for
   * about 210 ms.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void begin(uint32_t nowMs);

  /**
   * Sets the full touch delta as a share of the baseline, values at or below 0 are ignored.
   * @param {float} share - settings touchTrigger divided by 100, 0.15 until set
   * @returns {void}
   */
  void setTouchShare(float share);

  /**
   * Enables the lab CSV lines: s per active sample, i every 500 ms while idle, g per gesture.
   * @param {bool} enabled - true prints the lines
   * @returns {void}
   */
  void setDebug(bool enabled);

  /**
   * Rebases both coins now and once more a second later after the USB power changed, the ground
   * reference moves every reading at once and would otherwise read as a two-finger hold. Ends any
   * running gesture without reporting it, a running swipe ends incomplete.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void powerChanged(uint32_t nowMs);

  /**
   * Samples both coins once the sample interval elapsed and advances the gesture.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {FingerEvent} - the oldest waiting gesture, NONE in most calls
   */
  FingerEvent tick(uint32_t nowMs);

  /**
   * Returns the filtered finger column.
   * @returns {float} - column 0 to 15
   */
  float fingerX() const;

  /**
   * Returns the dome level of a coin. During a press with a fixed start only the start coin
   * reports one unless two fingers hold both coins, both report 0 while swiping and in the
   * settle after a swipe.
   * @param {Coin} coin - coin to ask about
   * @returns {uint8_t} - level 0 to 3
   */
  uint8_t level(Coin coin) const;

  /**
   * Returns whether a swipe is running.
   * @returns {bool} - true between SWIPE_BEGIN and SWIPE_END
   */
  bool swiping() const;

  /**
   * Returns the swipe disc radius, grown out of the start coin's dome and following the finger
   * only in the swipe direction. Kept at its last value after SWIPE_END.
   * @returns {float} - radius in doubled units around the start coin centre
   */
  float swipeRadius() const;

  /**
   * Returns the dome radius the swipe disc grew out of.
   * @returns {float} - radius in doubled units around the start coin centre
   */
  float swipeFloor() const;

  /**
   * Returns how long the running two-finger hold lasts.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {uint32_t} - milliseconds since the two-finger hold started, 0 otherwise
   */
  uint32_t bothHeldMs(uint32_t nowMs) const;

private:
  enum class Phase : uint8_t { IDLE, PRESS, SWIPE };

  static constexpr uint8_t kNearSamples = 2;
  /** delay of the second rebase after a power change, the reading settles over about a second */
  static constexpr uint32_t kRebaseFollowUpMs = 1000;
  static constexpr uint8_t kGoneSamples = 4;
  /** strength a contact press ends below, a finger hovering between taps stays above it */
  static constexpr float kContactRelease = 0.35f;
  static constexpr uint8_t kContactGoneSamples = 2;

  /**
   * Samples both coins, then confirms waiting taps, advances the phases and prints debug lines.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void sample(uint32_t nowMs);

  /**
   * Counts the near and gone samples and advances the phase.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void update(uint32_t nowMs);

  /**
   * Starts a press at the current column and coin strengths.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void startPress(uint32_t nowMs);

  /**
   * Feeds the running press, releases it once gone and turns it into a swipe when ready.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @param {bool} gone - the finger left, or a single contact fell below kContactRelease
   * @returns {void}
   */
  void trackPress(uint32_t nowMs, bool gone);

  /**
   * Classifies a released press as chord, tap, hold or nothing and reports it.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void releasePress(uint32_t nowMs);

  /**
   * Starts the swipe disc at the start coin's dome radius and reports SWIPE_BEGIN.
   * @returns {void}
   */
  void beginSwipe();

  /**
   * Moves the disc with the finger, ends the swipe on a two-finger hold or once the finger is
   * gone, reporting SWIPE_END.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @param {bool} gone - the finger stayed below kActiveShare for kGoneSamples
   * @returns {void}
   */
  void trackSwipe(uint32_t nowMs, bool gone);

  /**
   * Marks the running press as chorded once a two-finger hold runs and drops both tap windows.
   * @returns {void}
   */
  void markChord();

  /**
   * Returns to idle and starts the settle after a swipe, hold or chord release.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @param {bool} hideLevels - true reports dome level 0 for both coins during the settle
   * @returns {void}
   */
  void endGesture(uint32_t nowMs, bool hideLevels);

  /**
   * Reports TAP_CONFIRMED for every coin whose tap window passed without a press on it.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void confirmTaps(uint32_t nowMs);

  /**
   * Queues a gesture for tick() to hand out.
   * @param {FingerGesture} gesture - gesture to report
   * @param {Coin} coin - coin the gesture started on
   * @param {bool} complete - SWIPE_END only, released past the canvas middle
   * @param {uint32_t} heldMs - HOLD only, contact length
   * @returns {void}
   */
  void emit(FingerGesture gesture, Coin coin, bool complete = false, uint32_t heldMs = 0);

  /**
   * Records the summary line of the released press while debug is on.
   * @param {const char*} kind - gesture name of the release
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void logGesture(const char* kind, uint32_t nowMs);

  /**
   * Prints the sample line while debug is on, repeating the summaries while idle.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void logSample(uint32_t nowMs);

  FingerSensor sensor_;
  FingerPress press_;
  TapWindow taps_;
  ChordHold chord_;
  SwipeDisc disc_;
  FingerEventQueue events_;
  GestureLog log_;
  GestureSettle settle_;
  uint32_t lastSampleMs_ = 0;
  Phase phase_ = Phase::IDLE;
  uint8_t nearCount_ = 0;
  uint8_t goneCount_ = 0;
  uint8_t contactGoneCount_ = 0;
  /** millis() of the pending follow-up rebase, 0 when none is pending */
  uint32_t rebaseAtMs_ = 0;

  /**
   * Rebases the coins and drops the running gesture, the swipe ends incomplete.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void rebase(uint32_t nowMs);
  /** the running press met a two-finger hold and reports no gesture */
  bool chorded_ = false;
  bool debug_ = false;
};
