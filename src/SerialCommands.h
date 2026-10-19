#pragma once

#include "Commands.h"
#include "settings/Settings.h"

/**
 * Reads one line from Serial and turns it into a command: a state name, "b<0-15>" or "p<0|1>".
 * @param {Commands&} commands - queue the parsed state request is written to
 * @param {Settings&} settings - settings a brightness command writes through
 * @returns {void}
 */
void pollSerialCommands(Commands& commands, Settings& settings);
