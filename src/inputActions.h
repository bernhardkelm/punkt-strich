#pragma once

#include "TapMemory.h"
#include "touch/TouchEvent.h"

#include <cstdint>

class Settings;
class StateMachine;
struct Commands;

/**
 * Returns whether the shown state waits for a human, the button and the strich right coin dismiss
 * it instead of doing what they do otherwise.
 * @param {PunktState} shown - state currently on the matrix
 * @returns {bool} - true for ATTENTION, ERROR, LIMIT, WAITING and UNREAD
 */
bool waitsForAcknowledgement(PunktState shown);

/**
 * Takes the view change of the tap that opened the gesture back and darkens the matrix, or lights
 * it again. A tap that only woke the screen out of an automatic off is the whole gesture and
 * leaves the matrix alone.
 * @param {StateMachine&} state - state machine the gesture acts on
 * @param {TapMemory&} memory - what the tap did, cleared here
 * @param {uint32_t} nowMs - millis() at the time of the call
 * @returns {void}
 */
void applyDoubleTap(StateMachine& state, TapMemory& memory, uint32_t nowMs);

/**
 * Acknowledges the shown state or cycles the animation of the thinking activity.
 * @param {StateMachine&} state - state machine the shown state is read from and dismissed on
 * @param {Settings&} settings - settings the next thinking animation is written to
 * @param {Commands&} commands - flagged so loop() re-applies the settings
 * @param {uint32_t} nowMs - millis() at the time of the call
 * @returns {void}
 */
void handleButtonRelease(StateMachine& state, Settings& settings, Commands& commands,
                         uint32_t nowMs);

/**
 * Applies a touch gesture and remembers what a tap did, so the gesture that follows can finish or
 * take it back. A tap confirmation is handled first, acknowledges an ATTENTION that waited for it
 * and counts as no activity, every other gesture notes activity on the state machine. While the
 * screen is off from a timeout or the night schedule, any gesture only wakes it and returns.
 * Otherwise a tap cycles the view at once, defers the ATTENTION acknowledge to the confirmation
 * and is ignored while the view is OFF, a double tap restores the view the tap came from and
 * darkens the matrix or lights it again, a hold forces every session to IDLE.
 * @param {TouchEvent} event - gesture reported by Touch::tick()
 * @param {StateMachine&} state - state machine the gesture acts on
 * @param {TapMemory&} memory - what the last tap did, read and written here
 * @param {uint32_t} nowMs - millis() at the time of the call
 * @returns {void}
 */
void handleTouch(TouchEvent event, StateMachine& state, TapMemory& memory, uint32_t nowMs);
