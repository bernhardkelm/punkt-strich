#pragma once

#include <cstdint>

/** A percentage reported over the API, kept with the time it arrived so it can go stale. */
class ReportedPercent {
public:
  /**
   * Stores a report with its timestamp.
   * @param {uint8_t} percent - 0 to 100, larger values are capped
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void set(uint8_t percent, uint32_t nowMs) {
    percent_ = percent > 100 ? 100 : percent;
    receivedMs_ = nowMs;
    received_ = true;
  }

  /**
   * Returns the last report, without judging its age.
   * @returns {uint8_t} - 0 to 100, 0 when nothing was reported
   */
  uint8_t percent() const {
    return percent_;
  }

  /**
   * Returns whether a report ever arrived.
   * @returns {bool} - true after the first report, also once it went stale
   */
  bool received() const {
    return received_;
  }

  /**
   * Returns how long ago the report arrived.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {uint32_t} - milliseconds since the report, 0 when nothing was reported
   */
  uint32_t ageMs(uint32_t nowMs) const {
    if (!received_) {
      return 0;
    }

    return nowMs - receivedMs_;
  }

  /**
   * Returns whether the report is too old to show.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @param {uint32_t} staleMs - age from which a report no longer counts
   * @returns {bool} - true when nothing was reported or the report is at least staleMs old
   */
  bool stale(uint32_t nowMs, uint32_t staleMs) const {
    return !received_ || nowMs - receivedMs_ >= staleMs;
  }

  /**
   * Returns the value the animations should show.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @param {uint32_t} staleMs - age from which a report no longer counts
   * @returns {uint8_t} - the report, 0 while it is stale
   */
  uint8_t shown(uint32_t nowMs, uint32_t staleMs) const {
    return stale(nowMs, staleMs) ? 0 : percent_;
  }

private:
  uint8_t percent_ = 0;
  uint32_t receivedMs_ = 0;
  bool received_ = false;
};
