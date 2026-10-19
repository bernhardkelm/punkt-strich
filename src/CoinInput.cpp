#include "CoinInput.h"

#include "TapMemory.h"
#include "coinActions.h"
#include "overlay/TouchOverlay.h"
#include "state/stateNames.h"
#include "touch/Touch.h"
#include "touch/coinNames.h"

#include <Arduino.h>

CoinInput::CoinInput(FingerTracker& fingers, StateMachine& state, TouchOverlay& overlay,
                     TapMemory& memory)
    : fingers_(fingers),
      state_(state),
      overlay_(overlay),
      memory_(memory) {
}

void CoinInput::tick(uint32_t nowMs) {
  const FingerEvent event = fingers_.tick(nowMs);

  if (swipe_.restore && !overlay_.retracting()) {
    restoreSwipe(nowMs);
  }

  route(event, nowMs);
  followSwipe(nowMs);
}

uint8_t CoinInput::overlayLevel(Coin coin, uint32_t nowMs) const {
  const bool dismissWaits = coin == Coin::RIGHT && memory_.effect == TapEffect::ACK_WAITS &&
                            nowMs - ackTapMs_ < kAckDomeMs;

  if (dismissWaits) {
    return Touch::kContactLevel;
  }

  return fingers_.level(coin);
}

void CoinInput::route(const FingerEvent& event, uint32_t nowMs) {
  switch (event.gesture) {
    case FingerGesture::NONE:
      return;
    case FingerGesture::SWIPE_BEGIN:
      beginSwipe(event.coin, nowMs);
      return;
    case FingerGesture::SWIPE_END:
      endSwipe(event.complete, nowMs);
      return;
    case FingerGesture::TAP_CONFIRMED:
      // only a deferred dismiss changes the picture, a freeze would cut a running reveal short
      if (memory_.effect != TapEffect::ACK_WAITS) {
        handleCoinGesture(event, state_, memory_, nowMs);
        return;
      }

      runGesture(event, nowMs);
      return;
    default:
      runGesture(event, nowMs);
      return;
  }
}

void CoinInput::runGesture(const FingerEvent& input, uint32_t nowMs) {
  // restored before the freeze, the new frozen frame would keep the swiped view otherwise
  if (swipe_.restore) {
    restoreSwipe(nowMs);
  }

  const char* before = state_.animationName();
  overlay_.gestureBegins();
  const RippleScope scope = handleCoinGesture(input, state_, memory_, nowMs);
  overlay_.gestureEnds(state_.animationName() != before, input.coin, scope, nowMs);

  if (input.gesture == FingerGesture::TAP && memory_.effect == TapEffect::ACK_WAITS) {
    ackTapMs_ = nowMs;
  }
}

void CoinInput::beginSwipe(Coin from, uint32_t nowMs) {
  if (swipe_.restore) {
    restoreSwipe(nowMs);
  }

  if (startCoinSwipe(from, state_, memory_, nowMs) != SwipeStart::SWITCH) {
    return;
  }

  swipe_.from = state_.view();
  swipe_.to = swipeTarget(from, swipe_.from);
  swipe_.seenMs = nowMs;
  swipe_.running = true;
  swipe_.disc = overlay_.swipeBegins(from, fingers_.swipeFloor());

  if (swipe_.disc) {
    state_.setView(swipe_.to, nowMs);
  }

  Serial.printf("punkt: touch swipe from %s, view %s\n", coinName(from), viewName(swipe_.to));
}

void CoinInput::followSwipe(uint32_t nowMs) {
  if (!swipe_.running) {
    return;
  }

  if (fingers_.swiping()) {
    swipe_.seenMs = nowMs;
    overlay_.swipeTo(fingers_.swipeRadius());
    return;
  }

  if (nowMs - swipe_.seenMs > kSwipeLostMs) {
    Serial.println("punkt: touch swipe ended without its release");
    endSwipe(false, nowMs);
  }
}

void CoinInput::endSwipe(bool complete, uint32_t nowMs) {
  if (!swipe_.running) {
    return;
  }

  swipe_.running = false;
  Serial.printf("punkt: touch swipe %s\n", complete ? "complete" : "short, taking it back");

  if (!swipe_.disc) {
    if (complete) {
      state_.setView(swipe_.to, nowMs);
    }

    return;
  }

  overlay_.swipeEnds(complete, nowMs);
  swipe_.restore = !complete;
}

void CoinInput::restoreSwipe(uint32_t nowMs) {
  swipe_.restore = false;

  // a view another source changed in the meantime is left alone
  if (state_.view() == swipe_.to) {
    state_.setView(swipe_.from, nowMs);
    Serial.printf("punkt: touch swipe taken back, view %s\n", viewName(state_.view()));
  }

  overlay_.endSwipe();
}
