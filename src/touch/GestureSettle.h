#pragma once

#include <cstdint>

/**
 * Quiet period after a swipe, a hold or a chord release: no press may start for kSettleMs, and
 * after a swipe the dome levels read 0 meanwhile. A tap starts no settle.
 */
class GestureSettle {
public:
  /**
   * Starts the settle at the release of a swipe, a hold or a chord.
   * @param {uint32_t} nowMs - millis() of the release
   * @param {bool} hideLevels - true hides the dome levels until the settle ends
   * @returns {void}
   */
  void start(uint32_t nowMs, bool hideLevels);

  /**
   * Ends the settle once kSettleMs passed since its start.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void update(uint32_t nowMs);

  /**
   * Ends the settle at once, used when the tracker restarts.
   * @returns {void}
   */
  void reset();

  /**
   * Returns whether the settle still holds the next press back.
   * @returns {bool} - true from start() until update() ended it
   */
  bool settling() const;

  /**
   * Returns whether the dome levels are hidden.
   * @returns {bool} - true while a settle started with hideLevels runs
   */
  bool levelsHidden() const;

private:
  static constexpr uint32_t kSettleMs = 150;

  uint32_t startMs_ = 0;
  bool settling_ = false;
  bool levelsHidden_ = false;
};
