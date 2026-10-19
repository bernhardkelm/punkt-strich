#pragma once

#include "state/State.h"

#include <cstdint>

/**
 * Returns how strongly a state claims the matrix while several sessions share it.
 * @param {PunktState} state - state to rank
 * @returns {uint8_t} - higher wins, LOADING highest and IDLE lowest
 */
uint8_t statePriority(PunktState state);

/**
 * Returns whether a state counts as busy for the watchdog that resets a stuck session.
 * @param {PunktState} state - state to test
 * @returns {bool} - true for every state a session is expected to leave again
 */
bool isBusyState(PunktState state);
