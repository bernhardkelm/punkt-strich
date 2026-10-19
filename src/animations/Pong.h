#pragma once

#include "Animation.h"

/**
 * Pong on 8x8: a three pixel paddle in the first and the last column, a ball crossing the six
 * columns between them and bouncing off the top and bottom rows. The paddle the ball comes at
 * tracks it one row per step and only rarely hesitates, so a rally runs for a while before it
 * ends, the column that let the ball through flashes, both paddles come back to the middle, and
 * the next ball is served towards that side.
 */
class Pong : public Animation {
public:
  const char* name() const override;
  void begin(Display& display) override;
  void tick(Display& display, uint32_t nowMs) override;

private:
  static constexpr uint32_t kStepMs = 90;
  static constexpr uint32_t kScoreFlashMs = 120;
  static constexpr uint32_t kServePauseMs = 350;
  static constexpr uint8_t kScoreFlashes = 6;
  static constexpr uint8_t kHesitatePercent = 4;
  static constexpr int8_t kPaddleHeight = 3;
  static constexpr int8_t kLeftColumn = 0;
  static constexpr int8_t kRightColumn = Display::kWidth - 1;
  static constexpr int8_t kMaxTop = Display::kHeight - kPaddleHeight;
  static constexpr int8_t kRestTop = kMaxTop / 2;

  /** What the match is doing, only PLAY moves the ball. */
  enum class Phase : uint8_t { PLAY, SCORE };

  int8_t ballX_ = 1;
  int8_t ballY_ = 3;
  int8_t dx_ = 1;
  int8_t dy_ = 1;
  int8_t leftTop_ = kRestTop;
  int8_t rightTop_ = kRestTop;
  /** column of the paddle that let the last ball through, the one the score flashes on */
  int8_t loserColumn_ = kLeftColumn;
  uint8_t flashes_ = 0;
  uint32_t nextStepMs_ = 0;
  Phase phase_ = Phase::PLAY;

  /**
   * Limits a paddle's top row to the rows a full paddle fits in.
   * @param {int8_t} top - raw top row
   * @returns {int8_t} - 0 to kMaxTop
   */
  static int8_t clampTop(int8_t top);

  /**
   * Returns whether a paddle covers a row.
   * @param {int8_t} top - top row of the paddle
   * @param {int8_t} y - row the ball is on
   * @returns {bool} - true when the ball is returned
   */
  static bool covers(int8_t top, int8_t y);

  /**
   * Moves one paddle a row towards a target, unless it hesitates this step.
   * @param {int8_t} top - top row of the paddle
   * @param {int8_t} targetTop - top row it aims for
   * @param {bool} tracking - true while the ball comes at this paddle
   * @returns {int8_t} - top row after the step
   */
  static int8_t movePaddle(int8_t top, int8_t targetTop, bool tracking);

  /**
   * Lets both paddles follow the ball, the one it moves away from drifts back to the middle.
   * Ball and paddle both cover one row per step, so the tracking paddle reaches every row the
   * ball can arrive on and a miss only ever comes from a hesitation.
   * @returns {void}
   */
  void movePaddles();

  /**
   * Drifts both paddles a row back towards the middle, run on every score flash so a serve
   * starts from a row the paddle can still track the ball from.
   * @returns {void}
   */
  void restPaddles();

  /**
   * Moves the ball one step, bounces it off the rows and the paddles and scores a miss.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void moveBall(uint32_t nowMs);

  /**
   * Starts the flashing of the column the ball went past.
   * @param {int8_t} column - column of the paddle that missed
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void score(int8_t column, uint32_t nowMs);

  /**
   * Runs the score flashes and serves the next ball once they are over.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void flashScore(uint32_t nowMs);

  /**
   * Puts the ball in the middle heading towards the side that missed.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void serve(uint32_t nowMs);

  /**
   * Draws both paddles, the ball, and the lit column of a score flash.
   * @param {Display&} display - matrix to draw on
   * @returns {void}
   */
  void draw(Display& display) const;

  /**
   * Lights the pixels of one paddle.
   * @param {Display&} display - matrix to draw on
   * @param {int8_t} column - column the paddle stands in
   * @param {int8_t} top - top row of the paddle
   * @returns {void}
   */
  static void drawPaddle(Display& display, int8_t column, int8_t top);
};
