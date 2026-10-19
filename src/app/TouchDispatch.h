#pragma once

#include "touch/TouchEvent.h"

#include <cstdint>

class CoinInput;
class FingerTracker;
class StateMachine;
class Touch;
class TouchOverlay;
struct TapMemory;

/**
 * Ticks the touch input of either board: punkt's coin runs its gestures through handleTouch(),
 * strich's two coins through CoinInput on the finger tracker. Feeds the overlay its hover levels
 * and the reset hold its coin hold.
 */
class TouchDispatch {
public:
  /**
   * Binds the dispatch to the coins and everything a gesture acts on, nothing is touched yet.
   * @param {Touch&} touch - punkt's only coin, the left coin on strich
   * @param {Touch&} rightTouch - strich's right coin, never begun on punkt
   * @param {FingerTracker&} fingers - gesture source of strich's two coins
   * @param {CoinInput&} coinInput - gesture handling of strich's two coins
   * @param {TouchOverlay&} overlay - overlay that freezes, ripples and draws the domes
   * @param {TapMemory&} memory - what the last tap did
   * @param {StateMachine&} state - state machine the gestures act on
   */
  TouchDispatch(Touch& touch, Touch& rightTouch, FingerTracker& fingers, CoinInput& coinInput,
                TouchOverlay& overlay, TapMemory& memory, StateMachine& state);

  /**
   * Ticks the touch input: punkt's coin runs through runGesture(), strich's two coins through
   * CoinInput on the finger tracker. Both electrodes of strich keep ticking for the wake threshold,
   * /state and their logs, their gestures are dropped.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void tick(uint32_t nowMs);

  /**
   * Returns the coin hold the reset fill runs on: punkt's coin, or both strich coins held together,
   * a single strich coin never resets.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {uint32_t} - milliseconds held, 0 while released
   */
  uint32_t resetHeldMs(uint32_t nowMs) const;

  /**
   * Drives the overlay with the hover level of every coin, punkt has no right coin.
   * @param {uint8_t} ceiling - brightness ceiling last handed to the state machine
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void tickOverlay(uint8_t ceiling, uint32_t nowMs);

private:
  /**
   * Top hover level Touch::proximity() reports, the dome is held at it while an acknowledge waits.
   */
  static constexpr uint8_t kAckHoldLevel = 3;

  /**
   * Runs a touch gesture through handleTouch() wrapped in the overlay's freeze and reveal, so a
   * gesture that swaps the picture ripples into it instead of cutting over at once. NONE is
   * skipped, handleTouch() would return immediately for it anyway.
   * @param {TouchEvent} event - gesture reported by Touch::tick()
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void runGesture(TouchEvent event, uint32_t nowMs);

  /**
   * Returns the hover level the overlay draws punkt's dome from: a tap that waits for its double
   * tap window before it acknowledges keeps the dome up, every other gesture lets it follow the
   * finger.
   * @returns {uint8_t} - hover level 0 to 3
   */
  uint8_t overlayLevel() const;

  Touch& touch_;
  Touch& rightTouch_;
  FingerTracker& fingers_;
  CoinInput& coinInput_;
  TouchOverlay& overlay_;
  TapMemory& memory_;
  StateMachine& state_;
};
