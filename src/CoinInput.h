#pragma once

#include "state/State.h"
#include "touch/Coin.h"
#include "touch/FingerTracker.h"

#include <cstdint>

class TouchOverlay;
struct TapMemory;

/**
 * Runs the two coins of strich on the finger tracker: taps, double taps, confirmations and holds
 * go through handleCoinGesture() wrapped in the overlay's freeze and ripple. A swipe switches the
 * view at once beneath a disc that follows the finger, a short one takes the change back once the
 * disc shrank back.
 */
class CoinInput {
public:
  /**
   * Binds the input to the tracker and everything a gesture acts on, nothing is touched yet.
   * @param {FingerTracker&} fingers - tracker of both coins, begun by the caller
   * @param {StateMachine&} state - state machine the gestures act on
   * @param {TouchOverlay&} overlay - overlay that freezes, ripples and draws the swipe disc
   * @param {TapMemory&} memory - what the last tap did, shared by both coins
   */
  CoinInput(FingerTracker& fingers, StateMachine& state, TouchOverlay& overlay, TapMemory& memory);

  /**
   * Ticks the tracker, applies the gesture it produced and moves a running swipe disc.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void tick(uint32_t nowMs);

  /**
   * Returns the hover level the overlay draws a coin's dome from, the right dome is held at the
   * contact level while its tap's dismiss waits for the double tap window.
   * @param {Coin} coin - coin whose dome is drawn
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {uint8_t} - hover level 0 to 3
   */
  uint8_t overlayLevel(Coin coin, uint32_t nowMs) const;

private:
  /** longest the right dome is held for a deferred dismiss, the confirmation ends it sooner */
  static constexpr uint32_t kAckDomeMs = 500;
  /** time the tracker may stop swiping without a SWIPE_END before the swipe counts as short */
  static constexpr uint32_t kSwipeLostMs = 100;

  /** The swipe whose view change the disc reveals. */
  struct Swipe {
    View from = View::CLOCK;
    View to = View::CLOCK;
    /** millis() the tracker last reported the finger swiping */
    uint32_t seenMs = 0;
    /** switched or about to switch the view, its SWIPE_END did not arrive yet */
    bool running = false;
    /** the view switched at once beneath the disc, without it only a complete release switches */
    bool disc = false;
    /** a short swipe waits for its disc to retract before the view is restored */
    bool restore = false;
  };

  /**
   * Hands a gesture to the swipe handling or to handleCoinGesture().
   * @param {const FingerEvent&} event - gesture FingerTracker::tick() reported
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void route(const FingerEvent& event, uint32_t nowMs);

  /**
   * Applies one gesture inside the overlay's freeze and ripple.
   * @param {const FingerEvent&} input - tap, double tap, hold or confirmation
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void runGesture(const FingerEvent& input, uint32_t nowMs);

  /**
   * Starts the disc of an accepted swipe and switches the view beneath it.
   * @param {Coin} from - coin the swipe started on
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void beginSwipe(Coin from, uint32_t nowMs);

  /**
   * Moves the disc of a running swipe to the finger, ends a swipe whose SWIPE_END got lost.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void followSwipe(uint32_t nowMs);

  /**
   * Releases the disc, a short swipe restores its view once the disc retracted.
   * @param {bool} complete - true when the swipe keeps the new view
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void endSwipe(bool complete, uint32_t nowMs);

  /**
   * Restores the view a short swipe left, beneath the still frozen picture, and ends its reveal.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void restoreSwipe(uint32_t nowMs);

  FingerTracker& fingers_;
  StateMachine& state_;
  TouchOverlay& overlay_;
  TapMemory& memory_;
  Swipe swipe_;
  /** millis() of the right tap whose dismiss waits */
  uint32_t ackTapMs_ = 0;
};
