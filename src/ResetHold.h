#pragma once

#include <cstdint>

class Settings;
class StateMachine;

/** Turns a long BOOT button or touch hold into a fill on the matrix and finally a factory reset. */
class ResetHold {
public:
  /** Hold length after which the fill starts, shorter presses leave the matrix alone. */
  static constexpr uint32_t kArmMs = 2000;
  /** Button press released under this length is a click, between it and kArmMs nothing happens. */
  static constexpr uint32_t kClickMaxMs = 1000;
  /** Hold length at which the matrix is full and the reset fires. */
  static constexpr uint32_t kResetMs = 10000;

  ResetHold(StateMachine& state, Settings& settings);

  /**
   * Maps the current hold length onto the fill and erases everything once it is complete.
   * @param {uint32_t} heldMs - how long the button or the electrode has been held, 0 while released
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void tick(uint32_t heldMs, uint32_t nowMs);

private:
  /** Delay after the erase that restarts even while the input still reads as held. */
  static constexpr uint32_t kRestartTimeoutMs = 5000;

  /**
   * Restarts on the release and on kRestartTimeoutMs, a stuck electrode would otherwise leave the
   * board erased and parked.
   * @param {uint32_t} heldMs - how long the button or the electrode has been held, 0 while released
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void waitForRestart(uint32_t heldMs, uint32_t nowMs);

  /**
   * Erases the settings and the Wi-Fi credentials, the restart waits for the release because
   * GPIO0 held low through a reset would put the chip into the ROM bootloader.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void factoryReset(uint32_t nowMs);

  StateMachine& state_;
  Settings& settings_;
  uint32_t erasedMs_ = 0;
  bool armed_ = false;
  bool erased_ = false;
};
