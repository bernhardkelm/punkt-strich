#pragma once

#include "Animation.h"

/** Falling streaks, one per column, each with its own trail length and step interval. */
class MatrixRain : public Animation {
public:
  const char* name() const override;
  void begin(Display& display) override;
  void tick(Display& display, uint32_t nowMs) override;

private:
  static constexpr uint8_t kActiveColumns = Display::kWidth;
  static constexpr uint8_t kMinTrailLength = 2;
  static constexpr uint8_t kMaxTrailLength = 4;
  static constexpr uint32_t kBaseStepMs = 300;
  static constexpr uint8_t kStepJitterPercent = 40;
  static constexpr uint8_t kRestartPercent = 6;
  static constexpr uint32_t kFrameMs = 50;

  /** One streak, head is its lowest lit row and length the lit rows directly above it. */
  struct Column {
    bool running;
    uint8_t head;
    uint8_t length;
    uint32_t stepMs;
    uint32_t nextStepMs;
  };

  Column columns_[Display::kWidth] = {};
  uint32_t nextFrameMs_ = 0;

  /**
   * Starts a streak at the top row with a fresh trail length and step interval.
   * @param {Column&} column - column to restart
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void startColumn(Column& column, uint32_t nowMs) const;

  /**
   * Moves every running streak whose step is due one row down and ends it once its last trail
   * pixel left the bottom row.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {bool} - true when a streak moved
   */
  bool stepColumns(uint32_t nowMs);

  /**
   * Restarts idle columns with kRestartPercent probability each, so the streaks drift apart.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {bool} - true when a streak started
   */
  bool rollRestarts(uint32_t nowMs);

  /**
   * Draws the head and trail of every running streak.
   * @param {Display&} display - matrix to draw on
   * @returns {void}
   */
  void draw(Display& display) const;
};
