#include "inputActions.h"

#include "Commands.h"
#include "settings/Settings.h"
#include "settings/workingAnimationNames.h"
#include "state/State.h"
#include "state/stateNames.h"

#include <Arduino.h>

namespace {

/**
 * Finishes the tap whose double tap window just closed, the deferred ATTENTION acknowledge is the
 * only effect left to apply. Counts as activity only when it acknowledges, so a confirmation never
 * wakes the screen on its own.
 * @param {StateMachine&} state - state machine the acknowledge acts on
 * @param {TapMemory&} memory - what the tap did, cleared here
 * @param {uint32_t} nowMs - millis() at the time of the call
 * @returns {void}
 */
void confirmTap(StateMachine& state, TapMemory& memory, uint32_t nowMs) {
  if (memory.effect == TapEffect::ACK_WAITS && state.current() == PunktState::ATTENTION) {
    state.noteActivity(nowMs);
    Serial.println("punkt: touch tap, acknowledging attention");
    state.acknowledgeDisplayed(nowMs);
  }

  memory.effect = TapEffect::NONE;
}

/**
 * Acts on a tap right away and remembers what it did, so a double tap can take it back. The
 * ATTENTION acknowledge is the one effect nothing can undo, it waits for the confirmation instead.
 * @param {StateMachine&} state - state machine the tap acts on
 * @param {TapMemory&} memory - what the tap did, written here
 * @param {uint32_t} nowMs - millis() at the time of the call
 * @returns {void}
 */
void applyTap(StateMachine& state, TapMemory& memory, uint32_t nowMs) {
  if (state.current() == PunktState::ATTENTION) {
    memory.effect = TapEffect::ACK_WAITS;
    Serial.println("punkt: touch tap, attention waits for the double tap window");
    return;
  }

  if (state.view() == View::OFF) {
    Serial.println("punkt: touch tap ignored, screen off");
    memory.effect = TapEffect::NONE;
    return;
  }

  // a refused change leaves the view as it is, which makes the restore a no-op
  memory.viewBefore = state.view();
  memory.effect = TapEffect::SWITCHED_VIEW;
  state.toggleView(nowMs);
  Serial.printf("punkt: touch tap, view %s\n", viewName(state.view()));
}

}

bool waitsForAcknowledgement(PunktState shown) {
  return shown == PunktState::ATTENTION || shown == PunktState::ERROR ||
         shown == PunktState::LIMIT || shown == PunktState::WAITING ||
         shown == PunktState::UNREAD;
}

void applyDoubleTap(StateMachine& state, TapMemory& memory, uint32_t nowMs) {
  if (memory.effect == TapEffect::WOKE) {
    Serial.println("punkt: touch double tap, already the wake");
    memory.effect = TapEffect::NONE;
    return;
  }

  // the restore runs before toggleOff(), so the view it remembers is the one the gesture started on
  if (memory.effect == TapEffect::SWITCHED_VIEW) {
    state.setView(memory.viewBefore, nowMs);
  }

  state.toggleOff(nowMs);
  Serial.printf("punkt: touch double tap, view %s\n", viewName(state.view()));
  memory.effect = TapEffect::NONE;
}

void handleButtonRelease(StateMachine& state, Settings& settings, Commands& commands,
                         uint32_t nowMs) {
  const PunktState shown = state.current();

  if (waitsForAcknowledgement(shown)) {
    Serial.printf("punkt: button dismiss %s\n", stateName(shown));
    state.dismissDisplayed(nowMs);
    return;
  }

  const uint8_t current = static_cast<uint8_t>(settings.activityAnimation(Activity::THINKING));
  const auto next = static_cast<WorkingAnimation>((current + 1) % kWorkingAnimationCount);
  settings.setActivityAnimation(Activity::THINKING, next);
  commands.settingsChanged = true;
  Serial.printf("punkt: button thinking=%s\n", workingAnimationName(next));
}

void handleTouch(TouchEvent event, StateMachine& state, TapMemory& memory, uint32_t nowMs) {
  if (event == TouchEvent::NONE) {
    return;
  }

  if (event == TouchEvent::TAP_CONFIRMED) {
    confirmTap(state, memory, nowMs);
    return;
  }

  state.noteActivity(nowMs);

  const OffReason reason = state.offReason();

  if (reason == OffReason::TIMEOUT || reason == OffReason::SCHEDULE) {
    state.leaveAutoOff(nowMs);
    Serial.println("punkt: touch, screen on");
    memory.effect = (event == TouchEvent::TAP) ? TapEffect::WOKE : TapEffect::NONE;
    return;
  }

  if (event == TouchEvent::HOLD) {
    Serial.println("punkt: touch hold, forcing idle");
    state.forceIdle(nowMs);
    memory.effect = TapEffect::NONE;
    return;
  }

  if (event == TouchEvent::DOUBLE_TAP) {
    applyDoubleTap(state, memory, nowMs);
    return;
  }

  applyTap(state, memory, nowMs);
}
