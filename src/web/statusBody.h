#pragma once

#include <WString.h>

class Settings;
class StateMachine;
class Touch;

/**
 * Builds the GET /state body, every value is a POD read the HTTP task may touch.
 * @param {const StateMachine&} state - state machine the state, view, context and sessions come from
 * @param {const Settings&} settings - settings the power save and night sleep flags come from
 * @param {const Touch&} touch - punkt's coin or strich's left coin, reported as touch
 * @param {const Touch&} touchRight - strich's right coin, reported as touchRight on strich only
 * @returns {String} - serialised status object
 */
String statusBody(const StateMachine& state, const Settings& settings, const Touch& touch,
                  const Touch& touchRight);
