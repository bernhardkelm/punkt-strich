#include "coinActions.h"

#include "inputActions.h"
#include "state/State.h"
#include "state/stateNames.h"
#include "touch/Touch.h"
#include "touch/coinNames.h"

#include <Arduino.h>

namespace {

/**
 * Returns whether the matrix shows a state waiting for a human that the right tap dismisses:
 * ATTENTION and ERROR break through every view, the calm ones only show in NORMAL.
 * @param {const StateMachine&} state - state machine the shown state and the view are read from
 * @returns {bool} - true when the right tap defers a dismiss instead of switching the view
 */
bool showsWaitingState(const StateMachine& state) {
  const PunktState shown = state.current();

  if (!waitsForAcknowledgement(shown)) {
    return false;
  }

  if (shown == PunktState::ATTENTION || shown == PunktState::ERROR) {
    return true;
  }

  return state.view() == View::NORMAL;
}

/**
 * Finishes the right tap whose double tap window closed, the deferred dismiss is the only
 * effect left to apply. Counts as activity only when it dismisses, so a confirmation never
 * wakes the screen on its own.
 * @param {StateMachine&} state - state machine the dismiss acts on
 * @param {TapMemory&} memory - what the tap did, cleared here
 * @param {uint32_t} nowMs - millis() at the time of the call
 * @returns {void}
 */
void confirmCoinTap(StateMachine& state, TapMemory& memory, uint32_t nowMs) {
  if (memory.effect == TapEffect::ACK_WAITS && showsWaitingState(state)) {
    state.noteActivity(nowMs);
    Serial.printf("punkt: touch right tap, dismissing %s\n", stateName(state.current()));
    state.dismissDisplayed(nowMs);
  }

  memory.effect = TapEffect::NONE;
}

/**
 * Returns whether a single tap is ignored because the view is OFF, forgetting the last tap then.
 * @param {Coin} coin - coin that was tapped
 * @param {const StateMachine&} state - state machine the view is read from
 * @param {TapMemory&} memory - what the last tap did, cleared when the tap is ignored
 * @returns {bool} - true while the OFF view is on
 */
bool ignoredWhileOff(Coin coin, const StateMachine& state, TapMemory& memory) {
  if (state.view() != View::OFF) {
    return false;
  }

  Serial.printf("punkt: touch %s tap ignored, screen off\n", coinName(coin));
  memory.effect = TapEffect::NONE;
  return true;
}

/**
 * Lights the screen again when a timeout or the night schedule turned it off, the gesture that
 * did it does nothing else.
 * @param {Coin} coin - coin the gesture came from
 * @param {StateMachine&} state - state machine the screen is lit on
 * @param {uint32_t} nowMs - millis() at the time of the call
 * @returns {bool} - true when the gesture was the wake
 */
bool wokeFromAutoOff(Coin coin, StateMachine& state, uint32_t nowMs) {
  const OffReason reason = state.offReason();

  if (reason != OffReason::TIMEOUT && reason != OffReason::SCHEDULE) {
    return false;
  }

  state.leaveAutoOff(nowMs);
  Serial.printf("punkt: touch %s, screen on\n", coinName(coin));
  return true;
}

/**
 * Remembers the view a tap leaves so a double tap can restore it and switches to the next one, a
 * refused change leaves the view as it is, which makes that restore a no-op.
 * @param {View} next - view the tap enters
 * @param {Coin} coin - coin that was tapped
 * @param {StateMachine&} state - state machine the view is switched on
 * @param {TapMemory&} memory - what the tap did, written here
 * @param {uint32_t} nowMs - millis() at the time of the call
 * @returns {void}
 */
void switchView(View next, Coin coin, StateMachine& state, TapMemory& memory, uint32_t nowMs) {
  memory.viewBefore = state.view();
  memory.effect = TapEffect::SWITCHED_VIEW;
  state.setView(next, nowMs);
  Serial.printf("punkt: touch %s tap, view %s\n", coinName(coin), viewName(state.view()));
}

/**
 * Returns the view the left tap enters, the cycle CLOCK, NORMAL, BATTERY and back to CLOCK. The
 * split picture is skipped while it would be refused for lack of a session.
 * @param {const StateMachine&} state - state machine the tap acts on, the view is not OFF
 * @returns {View} - NORMAL from CLOCK, BATTERY from NORMAL and USAGE, CLOCK from BATTERY
 */
View leftTapTarget(const StateMachine& state) {
  switch (state.view()) {
    case View::CLOCK:
      return state.viewAllowed(View::NORMAL) ? View::NORMAL : View::BATTERY;
    case View::NORMAL:
    case View::USAGE:
      return View::BATTERY;
    case View::BATTERY:
    default:
      return View::CLOCK;
  }
}

/**
 * Cycles the clock, the split picture and the battery view, ignored while the view is OFF.
 * @param {StateMachine&} state - state machine the tap acts on
 * @param {TapMemory&} memory - what the tap did, written here
 * @param {uint32_t} nowMs - millis() at the time of the call
 * @returns {void}
 */
void applyLeftTap(StateMachine& state, TapMemory& memory, uint32_t nowMs) {
  if (ignoredWhileOff(Coin::LEFT, state, memory)) {
    return;
  }

  switchView(leftTapTarget(state), Coin::LEFT, state, memory, nowMs);
}

/**
 * Defers the dismiss of a shown state waiting for a human to the tap confirmation, otherwise
 * enters the split picture from the clock or the battery view or toggles NORMAL and USAGE inside
 * it.
 * @param {StateMachine&} state - state machine the tap acts on
 * @param {TapMemory&} memory - what the tap did, written here
 * @param {uint32_t} nowMs - millis() at the time of the call
 * @returns {RippleScope} - FULL when the tap enters the split picture from the clock or battery view, HALF otherwise
 */
RippleScope applyRightTap(StateMachine& state, TapMemory& memory, uint32_t nowMs) {
  if (showsWaitingState(state)) {
    memory.effect = TapEffect::ACK_WAITS;
    Serial.printf("punkt: touch right tap, %s waits for the double tap window\n",
                  stateName(state.current()));
    return RippleScope::HALF;
  }

  if (ignoredWhileOff(Coin::RIGHT, state, memory)) {
    return RippleScope::HALF;
  }

  const View view = state.view();

  if (view == View::CLOCK || view == View::BATTERY) {
    switchView(View::NORMAL, Coin::RIGHT, state, memory, nowMs);
    return RippleScope::FULL;
  }

  switchView(view == View::NORMAL ? View::USAGE : View::NORMAL, Coin::RIGHT, state, memory, nowMs);
  return RippleScope::HALF;
}

/**
 * Forces every session to IDLE on a right hold, a left hold does nothing.
 * @param {Coin} coin - coin that was held
 * @param {StateMachine&} state - state machine the hold acts on
 * @param {TapMemory&} memory - what the last tap did, cleared here
 * @param {uint32_t} nowMs - millis() at the time of the call
 * @returns {RippleScope} - FULL, the idle sessions return the split picture to the clock face
 */
RippleScope applyHold(Coin coin, StateMachine& state, TapMemory& memory, uint32_t nowMs) {
  memory.effect = TapEffect::NONE;

  if (coin == Coin::LEFT) {
    return RippleScope::FULL;
  }

  Serial.println("punkt: touch right hold, forcing idle");
  state.forceIdle(nowMs);
  return RippleScope::FULL;
}

}

RippleScope handleCoinGesture(const FingerEvent& input, StateMachine& state, TapMemory& memory,
                              uint32_t nowMs) {
  const FingerGesture gesture = input.gesture;

  if (gesture == FingerGesture::NONE || gesture == FingerGesture::SWIPE_BEGIN ||
      gesture == FingerGesture::SWIPE_END) {
    return RippleScope::FULL;
  }

  // a confirmed dismiss leaves the session idle, which returns the split picture to the clock face
  if (gesture == FingerGesture::TAP_CONFIRMED) {
    confirmCoinTap(state, memory, nowMs);
    return RippleScope::FULL;
  }

  if (gesture == FingerGesture::HOLD && input.heldMs >= Touch::kHoldMaxMs) {
    Serial.printf("punkt: touch %s hold of %u ms ignored\n", coinName(input.coin),
                  static_cast<unsigned>(input.heldMs));
    return RippleScope::FULL;
  }

  state.noteActivity(nowMs);

  if (wokeFromAutoOff(input.coin, state, nowMs)) {
    memory.effect = (gesture == FingerGesture::TAP) ? TapEffect::WOKE : TapEffect::NONE;
    return RippleScope::FULL;
  }

  if (gesture == FingerGesture::DOUBLE_TAP) {
    applyDoubleTap(state, memory, nowMs);
    return RippleScope::FULL;
  }

  if (gesture == FingerGesture::HOLD) {
    return applyHold(input.coin, state, memory, nowMs);
  }

  if (input.coin == Coin::RIGHT) {
    return applyRightTap(state, memory, nowMs);
  }

  applyLeftTap(state, memory, nowMs);
  return RippleScope::FULL;
}

View swipeTarget(Coin from, View view) {
  const bool split = view == View::NORMAL || view == View::USAGE;

  if (from == Coin::LEFT) {
    if (split) {
      return View::CLOCK;
    }

    return view == View::CLOCK ? View::BATTERY : view;
  }

  if (view == View::BATTERY) {
    return View::CLOCK;
  }

  if (view == View::USAGE) {
    return View::NORMAL;
  }

  return view == View::CLOCK ? View::NORMAL : view;
}

SwipeStart startCoinSwipe(Coin from, StateMachine& state, TapMemory& memory, uint32_t nowMs) {
  memory.effect = TapEffect::NONE;
  state.noteActivity(nowMs);

  if (wokeFromAutoOff(from, state, nowMs)) {
    return SwipeStart::IGNORED;
  }

  const View view = state.view();
  const View target = swipeTarget(from, view);

  if (view == View::OFF || view == target) {
    Serial.printf("punkt: touch swipe from %s ignored, view %s\n", coinName(from), viewName(view));
    return SwipeStart::IGNORED;
  }

  // a refused change keeps the view and flashes the glitch, the swipe shows no disc then
  if (!state.viewAllowed(target)) {
    state.setView(target, nowMs);
    return SwipeStart::IGNORED;
  }

  return SwipeStart::SWITCH;
}
