#include "touch/TapWindow.h"

namespace {

/**
 * Returns the slot of a coin in the window arrays.
 * @param {Coin} coin - coin to look up
 * @returns {uint8_t} - 0 for LEFT, 1 for RIGHT
 */
uint8_t slot(Coin coin) {
  return static_cast<uint8_t>(coin);
}

}

void TapWindow::drop(Coin coin) {
  pending_[slot(coin)] = false;
}

FingerGesture TapWindow::release(Coin coin, uint32_t pressStartMs, uint32_t releaseMs) {
  if (pending_[slot(coin)] && (pressStartMs - releaseMs_[slot(coin)]) < kDoubleTapMs) {
    pending_[slot(coin)] = false;
    return FingerGesture::DOUBLE_TAP;
  }

  releaseMs_[slot(coin)] = releaseMs;
  pending_[slot(coin)] = true;
  return FingerGesture::TAP;
}

bool TapWindow::confirm(Coin coin, bool pressRunning, uint32_t nowMs) {
  if (!pending_[slot(coin)] || pressRunning || (nowMs - releaseMs_[slot(coin)]) < kDoubleTapMs) {
    return false;
  }

  pending_[slot(coin)] = false;
  return true;
}
