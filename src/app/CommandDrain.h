#pragma once

#include <cstdint>

class Settings;
class SettingsApplier;
class StateMachine;
struct Commands;

/**
 * Applies the commands the HTTP task and the serial hook enqueued on the loop() side, and restarts
 * the board once a requested restart's grace period passed.
 */
class CommandDrain {
public:
  /**
   * Binds the drain to the queue and everything a command acts on, nothing is drained yet.
   * @param {Commands&} commands - queue the HTTP task and the serial hook write to
   * @param {Settings&} settings - settings store a settings patch is written to
   * @param {StateMachine&} state - state machine the state, report, view and preview requests reach
   * @param {SettingsApplier&} applier - hands changed settings to the running modules
   */
  CommandDrain(Commands& commands, Settings& settings, StateMachine& state,
               SettingsApplier& applier);

  /**
   * Applies the commands the HTTP task and the serial hook enqueued.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void drain(uint32_t nowMs);

private:
  static constexpr uint32_t kRestartGraceMs = 250;

  /**
   * Applies a POST /view request.
   * @param {int8_t} requested - View value or kViewToggle, -1 when nothing is pending
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void applyViewRequest(int8_t requested, uint32_t nowMs);

  /**
   * Arms the restart, the grace period gives a client that is still connected time to read the
   * response the HTTP task already handed to the network stack.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void armRestart(uint32_t nowMs);

  Commands& commands_;
  Settings& settings_;
  StateMachine& state_;
  SettingsApplier& applier_;
  uint32_t restartArmedMs_ = 0;
  bool restartPending_ = false;
};
