#include "SerialCommands.h"

#include "state/stateNames.h"

#include <Arduino.h>
#include <cstddef>

namespace {

constexpr size_t kLineMax = 24;

char lineBuffer[kLineMax] = {};
size_t lineLength = 0;

/**
 * Parses a "b<0-15>" brightness command.
 * @param {const char*} line - trimmed command line
 * @param {uint8_t&} out - receives the brightness level on success
 * @returns {bool} - true when the line was a valid brightness command
 */
bool parseBrightness(const char* line, uint8_t& out) {
  if (line[0] != 'b' || line[1] == '\0') {
    return false;
  }

  uint16_t value = 0;

  for (const char* digit = line + 1; *digit != '\0'; digit++) {
    if (*digit < '0' || *digit > '9') {
      return false;
    }

    value = static_cast<uint16_t>(value * 10 + (*digit - '0'));

    if (value > Settings::kMaxBrightness) {
      return false;
    }
  }

  out = static_cast<uint8_t>(value);
  return true;
}

/**
 * Parses a "p0" or "p1" power save command.
 * @param {const char*} line - trimmed command line
 * @param {bool&} out - receives the power save flag on success
 * @returns {bool} - true when the line was a valid power save command
 */
bool parsePowerSave(const char* line, bool& out) {
  if (line[0] != 'p' || (line[1] != '0' && line[1] != '1') || line[2] != '\0') {
    return false;
  }

  out = line[1] == '1';
  return true;
}

/**
 * Turns one complete line into a command.
 * @param {const char*} line - trimmed command line
 * @param {Commands&} commands - queue the parsed state request is written to
 * @param {Settings&} settings - settings a brightness command writes through
 * @returns {void}
 */
void apply(const char* line, Commands& commands, Settings& settings) {
  PunktState state = PunktState::IDLE;
  uint8_t level = 0;

  if (parseState(line, state)) {
    Serial.printf("punkt: serial state %s\n", line);
    commands.postState("", state, kDefaultActivity, -1, 0);
    return;
  }

  if (parseBrightness(line, level)) {
    Serial.printf("punkt: serial brightness=%u\n", level);
    settings.setBrightness(level);
    commands.settingsChanged = true;
    return;
  }

  bool powerSave = false;

  if (parsePowerSave(line, powerSave)) {
    Serial.printf("punkt: serial powerSave=%u\n", powerSave ? 1 : 0);
    settings.setPowerSave(powerSave);
    commands.settingsChanged = true;
    return;
  }

  Serial.printf("punkt: serial unknown command %s\n", line);
}

}

void pollSerialCommands(Commands& commands, Settings& settings) {
  while (Serial.available() > 0) {
    const char character = static_cast<char>(Serial.read());

    if (character == '\r') {
      continue;
    }

    if (character != '\n') {
      if (lineLength + 1 < kLineMax) {
        lineBuffer[lineLength++] = character;
      }

      continue;
    }

    lineBuffer[lineLength] = '\0';

    if (lineLength > 0) {
      apply(lineBuffer, commands, settings);
    }

    lineLength = 0;
  }
}
