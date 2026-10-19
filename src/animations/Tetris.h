#pragma once

#include "Animation.h"

/** Falling blocks stacking up, full rows flash and collapse, the stack resets at the top. */
class Tetris : public Animation {
public:
  const char* name() const override;
  void begin(Display& display) override;
  void tick(Display& display, uint32_t nowMs) override;

private:
  static constexpr uint32_t kStepMs = 110;
  static constexpr uint32_t kFlashToggleMs = 25;
  static constexpr uint8_t kFullRow = 0xFF;

  uint8_t stack_[Display::kHeight] = {};
  int8_t pieceX_ = 0;
  int8_t pieceY_ = 0;
  uint8_t pieceWidth_ = 1;
  uint8_t pieceHeight_ = 1;
  uint32_t nextStepMs_ = 0;
  uint32_t flashStartMs_ = 0;
  bool flashing_ = false;
  bool flashOn_ = false;

  /** Empties the stack. */
  void resetStack();

  /** Picks a random block size and column, resetting the stack when it no longer fits. */
  void spawnPiece();

  /** Writes the current piece into the stack. */
  void mergePiece();

  /** Drops full rows and shifts the rows above them down. */
  void collapseFullRows();

  /**
   * Returns the column mask the piece covers.
   * @param {int8_t} x - left edge of the piece
   * @returns {uint8_t} - one bit per covered column
   */
  uint8_t pieceMask(int8_t x) const;

  /**
   * Returns whether the piece at the given position hits the floor or the stack.
   * @param {int8_t} x - left edge of the piece
   * @param {int8_t} y - top edge of the piece
   * @returns {bool} - true on collision
   */
  bool collides(int8_t x, int8_t y) const;

  /**
   * Returns whether any row is completely filled.
   * @returns {bool} - true when at least one row is full
   */
  bool hasFullRows() const;

  /**
   * Draws the stack plus either the falling piece or the flashing full rows.
   * @param {Display&} display - matrix to draw on
   * @returns {void}
   */
  void draw(Display& display) const;
};
