#include "app/TouchDispatch.h"

#include "CoinInput.h"
#include "TapMemory.h"
#include "board/boardConfig.h"
#include "inputActions.h"
#include "overlay/TouchOverlay.h"
#include "state/State.h"
#include "touch/Coin.h"
#include "touch/FingerTracker.h"
#include "touch/Touch.h"

TouchDispatch::TouchDispatch(Touch& touch, Touch& rightTouch, FingerTracker& fingers,
                             CoinInput& coinInput, TouchOverlay& overlay, TapMemory& memory,
                             StateMachine& state)
    : touch_(touch),
      rightTouch_(rightTouch),
      fingers_(fingers),
      coinInput_(coinInput),
      overlay_(overlay),
      memory_(memory),
      state_(state) {
}

void TouchDispatch::tick(uint32_t nowMs) {
  if (!Board::kHasClockHalf) {
    runGesture(touch_.tick(nowMs), nowMs);
    return;
  }

  touch_.tick(nowMs);
  rightTouch_.tick(nowMs);
  coinInput_.tick(nowMs);
}

uint32_t TouchDispatch::resetHeldMs(uint32_t nowMs) const {
  if (Board::kHasClockHalf) {
    return fingers_.bothHeldMs(nowMs);
  }

  return touch_.heldMs(nowMs);
}

void TouchDispatch::tickOverlay(uint8_t ceiling, uint32_t nowMs) {
  if (Board::kHasClockHalf) {
    overlay_.tick(coinInput_.overlayLevel(Coin::LEFT, nowMs),
                  coinInput_.overlayLevel(Coin::RIGHT, nowMs), state_.systemPicture(), ceiling,
                  nowMs);
    return;
  }

  overlay_.tick(overlayLevel(), 0, state_.systemPicture(), ceiling, nowMs);
}

void TouchDispatch::runGesture(TouchEvent event, uint32_t nowMs) {
  if (event == TouchEvent::NONE) {
    return;
  }

  const char* before = state_.animationName();
  overlay_.gestureBegins();
  handleTouch(event, state_, memory_, nowMs);
  overlay_.gestureEnds(state_.animationName() != before, Coin::LEFT, RippleScope::FULL, nowMs);
}

uint8_t TouchDispatch::overlayLevel() const {
  if (memory_.effect == TapEffect::ACK_WAITS && touch_.tapPending()) {
    return kAckHoldLevel;
  }

  return touch_.proximity();
}
