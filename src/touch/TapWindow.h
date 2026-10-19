#pragma once

#include "touch/Coin.h"
#include "touch/FingerEvent.h"

#include <cstdint>

/**
 * Double tap windows of both coins. A reported tap opens the window of its coin; a tap on the
 * same coin whose press starts within kDoubleTapMs of that release pairs into a double tap,
 * otherwise the tap is confirmed once the window passed and no press on that coin is running.
 * A window dropped while open reports neither.
 */
class TapWindow {
public:
  /**
   * Closes a coin's window without a double tap or a confirmation.
   * @param {Coin} coin - coin whose window closes
   * @returns {void}
   */
  void drop(Coin coin);

  /**
   * Decides a released tap: it pairs with the coin's open tap when its press started inside that
   * window, which then closes, otherwise it opens the coin's window itself.
   * @param {Coin} coin - coin that was tapped
   * @param {uint32_t} pressStartMs - millis() the tap's press started
   * @param {uint32_t} releaseMs - millis() of the tap's release
   * @returns {FingerGesture} - DOUBLE_TAP when it paired, TAP otherwise
   */
  FingerGesture release(Coin coin, uint32_t pressStartMs, uint32_t releaseMs);

  /**
   * Closes a coin's window once it passed with no press on that coin running.
   * @param {Coin} coin - coin whose window is checked
   * @param {bool} pressRunning - a press on that coin is running and holds the confirmation back
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {bool} - true once, in the call the tap is confirmed
   */
  bool confirm(Coin coin, bool pressRunning, uint32_t nowMs);

private:
  static constexpr uint32_t kDoubleTapMs = 300;

  uint32_t releaseMs_[2] = {};
  bool pending_[2] = {};
};
