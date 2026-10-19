#pragma once

#include <cstddef>
#include <cstdint>

/**
 * Keeps the last kCount gesture summary lines and prints them all again every kRepeatMs while
 * the caller is idle, the host serial path drops whole bursts and a lost line arrives later.
 */
class GestureLog {
public:
  /** buffer size of one summary line including the terminator */
  static constexpr size_t kLineSize = 96;

  /**
   * Stores a summary line over the oldest one and prints it.
   * @param {const char*} line - summary line, cut at kLineSize - 1 characters
   * @returns {void}
   */
  void record(const char* line);

  /**
   * Prints every stored line again once kRepeatMs passed since the last repeat.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void repeat(uint32_t nowMs);

private:
  static constexpr uint8_t kCount = 8;
  static constexpr uint32_t kRepeatMs = 2000;

  char lines_[kCount][kLineSize] = {};
  uint8_t next_ = 0;
  uint32_t repeatMs_ = 0;
};
