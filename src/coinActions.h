#pragma once

#include "TapMemory.h"
#include "overlay/TouchOverlay.h"
#include "state/State.h"
#include "touch/Coin.h"
#include "touch/FingerTracker.h"

#include <cstdint>

/** What a recognised swipe leaves to its caller. */
enum class SwipeStart : uint8_t { IGNORED, SWITCH };

/**
 * Applies a gesture of the strich finger tracker and remembers what a tap did, so the gesture
 * that follows can finish or take it back. A tap confirmation is handled first and dismisses the
 * shown state waiting for a human that a right tap deferred to it. A hold released at or after
 * Touch::kHoldMaxMs does nothing. Every other gesture counts as activity, and while the screen is
 * off from a timeout or the night schedule it only wakes it. Otherwise a left tap cycles the
 * full clock, the split picture and the battery view, a right tap defers the dismiss of a shown
 * waiting state, enters the split picture from the clock or the battery view or toggles NORMAL
 * and USAGE inside it, single taps are ignored in the OFF view. A double tap on either coin
 * works as on punkt, a right hold forces every session to IDLE and a left hold does nothing.
 * Swipes go through startCoinSwipe() instead.
 * @param {const FingerEvent&} input - gesture FingerTracker::tick() reported
 * @param {StateMachine&} state - state machine the gesture acts on
 * @param {TapMemory&} memory - what the last tap did, read and written here
 * @param {uint32_t} nowMs - millis() at the time of the call
 * @returns {RippleScope} - HALF for a right tap that defers the dismiss or toggles NORMAL and USAGE, else FULL
 */
RippleScope handleCoinGesture(const FingerEvent& input, StateMachine& state, TapMemory& memory,
                              uint32_t nowMs);

/**
 * Returns the view a swipe leads to along the strip of battery view, full clock and split
 * picture: a swipe to the right moves one step toward the battery view, one to the left one step
 * toward the split picture, except that a swipe to the left in the usage view returns to the
 * normal view.
 * @param {Coin} from - coin the swipe started on, RIGHT for a swipe to the left
 * @param {View} view - view shown when the swipe began
 * @returns {View} - the next view along the strip, the shown view at its end and in OFF
 */
View swipeTarget(Coin from, View view);

/**
 * Decides a recognised swipe before its disc runs: it counts as activity and drops what the last
 * tap left waiting. While the screen is off from a timeout or the night schedule it only wakes
 * it, in the OFF view and at an end of the strip it is ignored, and without a named session the
 * state machine refuses a change into the split picture and flashes the glitch.
 * @param {Coin} from - coin the swipe started on
 * @param {StateMachine&} state - state machine the swipe acts on
 * @param {TapMemory&} memory - what the last tap did, cleared here
 * @param {uint32_t} nowMs - millis() at the time of the call
 * @returns {SwipeStart} - SWITCH when the caller switches to swipeTarget() under the disc
 */
SwipeStart startCoinSwipe(Coin from, StateMachine& state, TapMemory& memory, uint32_t nowMs);
