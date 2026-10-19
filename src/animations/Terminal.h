#pragma once

#include "Animation.h"

/**
 * A shell session scrolling up out of the bottom row: a command types out slowly on the input
 * line with a caret blinking ahead of it, one or two output lines follow much faster, and every
 * finished line is pushed into the history above, the oldest row falling off the top. A command
 * keeps a pixel in the leftmost column as it scrolls, so past commands are marked down the left
 * edge, and a blank row separates one block from the next. The prompt marker in the bottom left
 * corner stays lit throughout.
 */
class Terminal : public Animation {
public:
  const char* name() const override;
  void begin(Display& display) override;
  void tick(Display& display, uint32_t nowMs) override;

private:
  static constexpr uint32_t kCommandPixelMs = 70;
  static constexpr uint32_t kOutputPixelMs = 25;
  static constexpr uint32_t kCommandPauseMs = 250;
  static constexpr uint32_t kOutputPauseMs = 100;
  static constexpr uint32_t kCaretBlinkMs = 300;
  static constexpr uint32_t kPushPauseMs = 150;
  static constexpr uint8_t kMinLineLength = 1;
  static constexpr uint8_t kMaxLineLength = 6;
  static constexpr uint8_t kMaxOutputLines = 2;
  static constexpr uint8_t kPromptColumn = 0;
  static constexpr uint8_t kTextColumn = 2;
  static constexpr uint8_t kInputRow = static_cast<uint8_t>(Display::kHeight - 1);

  /** What the input line is doing, PAUSE and PUSH leave the line itself alone. */
  enum class Phase : uint8_t { TYPING, PAUSE, PUSH };

  uint8_t rows_[Display::kHeight] = {};
  uint8_t column_ = kTextColumn;
  uint8_t endColumn_ = kTextColumn;
  uint8_t outputLeft_ = 0;
  uint32_t nextStepMs_ = 0;
  uint32_t nextCaretMs_ = 0;
  Phase phase_ = Phase::TYPING;
  bool command_ = true;
  bool caretOn_ = true;
  /** the next push carries the blank row that closes the block */
  bool separatorPending_ = false;

  /**
   * Picks the length of the next line and empties the input row for it.
   * @returns {void}
   */
  void planLine();

  /**
   * Starts a command line, plans how many output lines follow it and lights the caret.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void startCommand(uint32_t nowMs);

  /**
   * Starts an output line, which types faster and carries no caret.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void startOutput(uint32_t nowMs);

  /**
   * Lights the next pixel of the input line and ends the line on its last column.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {bool} - true, the input line always changed
   */
  bool typeStep(uint32_t nowMs);

  /**
   * Holds the finished line for the pause its kind gets before it is pushed.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void endLine(uint32_t nowMs);

  /**
   * Marks a command line in the leftmost column and pushes the input line into the history.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {bool} - true, the whole picture moved up
   */
  bool pushLine(uint32_t nowMs);

  /**
   * Pushes the blank row closing a block, or starts the next output or command line.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {bool} - true when the picture changed
   */
  bool advance(uint32_t nowMs);

  /**
   * Shifts every row up by one, the oldest row falls off the top and the input row is emptied.
   * @returns {void}
   */
  void shiftUp();

  /**
   * Flips the caret when its blink is due.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {bool} - true when the flip changed the picture
   */
  bool updateCaret(uint32_t nowMs);

  /**
   * Returns whether the line being typed carries a caret at all.
   * @returns {bool} - true while a command line types
   */
  bool caretRuns() const;

  /**
   * Returns whether the caret is lit right now.
   * @returns {bool} - true on the lit half of the blink of a command line
   */
  bool caretShown() const;

  /**
   * Draws the history, the input line, the prompt marker and the caret.
   * @param {Display&} display - matrix to draw on
   * @returns {void}
   */
  void draw(Display& display) const;
};
