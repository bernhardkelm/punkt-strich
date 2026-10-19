#pragma once

#include "state/State.h"

/**
 * Returns the lowercase name of a state.
 * @param {PunktState} state - state to name
 * @returns {const char*} - "off", "loading", "idle", "working", "attention", "done", "error",
 *                          "waiting", "compacting", "limit", "unread" or "glitch"
 */
const char* stateName(PunktState state);

/**
 * Parses a lowercase state name.
 * @param {const char*} name - candidate name, may be null
 * @param {PunktState&} out - receives the state on success
 * @returns {bool} - true when the name was known
 */
bool parseState(const char* name, PunktState& out);

/**
 * Returns the lowercase name of a display view.
 * @param {View} view - view to name
 * @returns {const char*} - "clock", "normal", "usage", "off" or "battery"
 */
const char* viewName(View view);

/**
 * Parses a lowercase view name, "battery" included whether or not the board offers it.
 * @param {const char*} name - candidate name, may be null
 * @param {View&} out - receives the view on success
 * @returns {bool} - true when the name was known
 */
bool parseView(const char* name, View& out);

/**
 * Returns the lowercase name of an off reason.
 * @param {OffReason} reason - reason to name
 * @returns {const char*} - "none", "manual", "timeout" or "schedule"
 */
const char* offReasonName(OffReason reason);
