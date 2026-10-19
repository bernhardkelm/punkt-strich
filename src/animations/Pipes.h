#pragma once

#include "Animation.h"

/**
 * The 3D pipes screensaver reduced to one colour on 8x8: a pipe grows pixel by pixel along a path
 * that keeps a dark pixel between itself and every pipe already laid, including its own body, so no
 * two runs ever sit side by side. It only ever comes to rest in front of a pipe it may cross, and
 * then goes straight through it onto the pixel behind, never twice in a row and never through a
 * corner. A pipe that has run into a dead end is followed by the next one, and once no pixel is free
 * enough to start on the matrix is cleared and the run begins again.
 */
class Pipes : public Animation {
public:
  const char* name() const override;
  void begin(Display& display) override;
  void tick(Display& display, uint32_t nowMs) override;

private:
  static constexpr uint32_t kStepMs = 150;
  static constexpr uint8_t kStraightPercent = 75;
  static constexpr uint32_t kPipePauseMs = 400;
  static constexpr uint32_t kStartBlinkMs = 120;
  static constexpr uint8_t kStartBlinkSteps = 2;
  static constexpr uint32_t kFullHoldMs = 800;
  static constexpr uint8_t kDirectionCount = 4;

  /** What the run is doing, only GROWING moves the head. */
  enum class Phase : uint8_t { START_BLINK, GROWING, PAUSE, FULL_HOLD };

  uint8_t rows_[Display::kHeight] = {};
  uint8_t headX_ = 0;
  uint8_t headY_ = 0;
  uint8_t direction_ = 0;
  uint8_t startBlink_ = 0;
  bool crossedLast_ = false;
  uint32_t nextStepMs_ = 0;
  Phase phase_ = Phase::START_BLINK;

  /**
   * Returns the column step of a direction.
   * @param {uint8_t} direction - 0 up, 1 right, 2 down, 3 left
   * @returns {int8_t} - -1, 0 or 1
   */
  static int8_t stepX(uint8_t direction);

  /**
   * Returns the row step of a direction.
   * @param {uint8_t} direction - 0 up, 1 right, 2 down, 3 left
   * @returns {int8_t} - -1, 0 or 1
   */
  static int8_t stepY(uint8_t direction);

  /**
   * Returns whether a coordinate is on the matrix.
   * @param {int8_t} x - column, 0 is left
   * @param {int8_t} y - row, 0 is top
   * @returns {bool} - true when the pixel exists
   */
  static bool inGrid(int8_t x, int8_t y);

  /**
   * Returns whether a pixel is on the matrix and lit.
   * @param {int8_t} x - column, 0 is left
   * @param {int8_t} y - row, 0 is top
   * @returns {bool} - false for everything outside the matrix
   */
  bool isLit(int8_t x, int8_t y) const;

  /**
   * Returns whether a pixel is on the matrix and still dark.
   * @param {int8_t} x - column, 0 is left
   * @param {int8_t} y - row, 0 is top
   * @returns {bool} - false for everything outside the matrix
   */
  bool isDark(int8_t x, int8_t y) const;

  /**
   * Returns whether a pixel keeps the spacing rule, meaning none of its four orthogonal neighbours
   * is lit apart from the one cell the pipe arrives from. Diagonal neighbours do not count.
   * @param {int8_t} x - column of the pixel to test
   * @param {int8_t} y - row of the pixel to test
   * @param {int8_t} allowX - column of the one neighbour that may be lit, off the matrix for none
   * @param {int8_t} allowY - row of the one neighbour that may be lit, off the matrix for none
   * @returns {bool} - true when the pixel has the room a pipe needs
   */
  bool hasSpacing(int8_t x, int8_t y, int8_t allowX, int8_t allowY) const;

  /**
   * Returns whether both pixels beside a pixel, across the direction of travel, are dark. This is
   * what keeps two runs from ever going side by side and a pipe from folding back onto its body.
   * @param {int8_t} x - column of the pixel to test
   * @param {int8_t} y - row of the pixel to test
   * @param {uint8_t} direction - direction of travel the sides are taken across
   * @returns {bool} - true when neither side is lit
   */
  bool hasSideRoom(int8_t x, int8_t y, uint8_t direction) const;

  /**
   * Returns whether a pipe on a pixel may move one further in a direction. The target has to be
   * dark with both sides dark, and what lies straight beyond it has to be dark as well or a pipe the
   * next step can cross.
   * @param {int8_t} x - column the pipe stands on
   * @param {int8_t} y - row the pipe stands on
   * @param {uint8_t} direction - direction to move in
   * @returns {bool} - true when the move is allowed
   */
  bool canStep(int8_t x, int8_t y, uint8_t direction) const;

  /**
   * Returns whether a pipe on a pixel may cross the lit pixel straight ahead, which needs that pixel
   * to run perpendicular to the direction of travel and the pixel behind it to be dark with no lit
   * neighbour other than the crossed one. Says nothing about the step before, the caller keeps two
   * crossings in a row apart.
   * @param {int8_t} x - column the pipe stands on
   * @param {int8_t} y - row the pipe stands on
   * @param {uint8_t} direction - direction of travel, crossings are straight only
   * @returns {bool} - true when the crossing is allowed
   */
  bool canCross(int8_t x, int8_t y, uint8_t direction) const;

  /**
   * Returns whether a pipe may start on a pixel, which needs it dark, free of lit orthogonal
   * neighbours and holding at least one allowed first move.
   * @param {uint8_t} x - column to test
   * @param {uint8_t} y - row to test
   * @returns {bool} - true when a new pipe can be put there
   */
  bool canStartAt(uint8_t x, uint8_t y) const;

  /**
   * Picks a random pixel a pipe may start on and puts the head on it.
   * @returns {bool} - false when no pixel qualifies and the matrix has to be cleared
   */
  bool pickStart();

  /**
   * Starts the blink of a new pipe, or holds the full matrix when none can start.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void startPipe(uint32_t nowMs);

  /**
   * Runs the start blink and lets the pipe grow in a random allowed direction once it is over.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void blinkStart(uint32_t nowMs);

  /**
   * Moves the head one pixel, mostly straight on and otherwise around a corner, crossing a
   * perpendicular pipe where that is the only way ahead, and ends the pipe when nothing is allowed.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void growStep(uint32_t nowMs);

  /**
   * Turns the head onto a random allowed side and moves it there, and ends the pipe when neither
   * side is allowed and straight ahead is blocked as well.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @param {bool} straightAllowed - whether the head could still move straight ahead
   * @returns {bool} - true when the head turned and moved
   */
  bool turnOrEnd(uint32_t nowMs, bool straightAllowed);

  /**
   * Puts the head one pixel further in its direction and lights that pixel.
   * @returns {void}
   */
  void advance();

  /**
   * Puts the head two pixels further in its direction, over the crossed pipe onto the dark pixel
   * behind it, and lights only that pixel.
   * @returns {void}
   */
  void crossOver();

  /**
   * Ends the pipe and waits before the next one starts.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void endPipe(uint32_t nowMs);

  /**
   * Clears the matrix once the full hold is over and starts the next pipe.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void clearGrid(uint32_t nowMs);

  /**
   * Draws every laid pixel, with the start pixel following its blink.
   * @param {Display&} display - matrix to draw on
   * @returns {void}
   */
  void draw(Display& display) const;
};
