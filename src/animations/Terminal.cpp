#include "animations/Terminal.h"

#include <Arduino.h>

const char* Terminal::name() const {
  return "terminal";
}

void Terminal::begin(Display& display) {
  for (uint8_t& row : rows_) {
    row = 0;
  }

  separatorPending_ = false;
  startCommand(millis());
  draw(display);
}

void Terminal::tick(Display& display, uint32_t nowMs) {
  bool changed = updateCaret(nowMs);

  if (static_cast<int32_t>(nowMs - nextStepMs_) >= 0) {
    switch (phase_) {
      case Phase::TYPING:
        changed = typeStep(nowMs) || changed;
        break;
      case Phase::PAUSE:
        changed = pushLine(nowMs) || changed;
        break;
      case Phase::PUSH:
      default:
        changed = advance(nowMs) || changed;
        break;
    }
  }

  if (changed) {
    draw(display);
  }
}

void Terminal::planLine() {
  const uint8_t length =
      static_cast<uint8_t>(kMinLineLength + random(kMaxLineLength - kMinLineLength + 1));
  rows_[kInputRow] = 0;
  column_ = kTextColumn;
  endColumn_ = static_cast<uint8_t>(kTextColumn + length - 1);
}

void Terminal::startCommand(uint32_t nowMs) {
  planLine();
  command_ = true;
  outputLeft_ = static_cast<uint8_t>(1 + random(kMaxOutputLines));
  caretOn_ = true;
  nextCaretMs_ = nowMs + kCaretBlinkMs;
  phase_ = Phase::TYPING;
  nextStepMs_ = nowMs + kCommandPixelMs;
}

void Terminal::startOutput(uint32_t nowMs) {
  planLine();
  command_ = false;
  phase_ = Phase::TYPING;
  nextStepMs_ = nowMs + kOutputPixelMs;
}

bool Terminal::typeStep(uint32_t nowMs) {
  rows_[kInputRow] = static_cast<uint8_t>(rows_[kInputRow] | (1 << column_));

  if (column_ >= endColumn_) {
    endLine(nowMs);
    return true;
  }

  column_++;
  nextStepMs_ = nowMs + (command_ ? kCommandPixelMs : kOutputPixelMs);
  return true;
}

void Terminal::endLine(uint32_t nowMs) {
  phase_ = Phase::PAUSE;
  nextStepMs_ = nowMs + (command_ ? kCommandPauseMs : kOutputPauseMs);
}

bool Terminal::pushLine(uint32_t nowMs) {
  if (command_) {
    rows_[kInputRow] = static_cast<uint8_t>(rows_[kInputRow] | (1 << kPromptColumn));
  } else {
    outputLeft_--;
    separatorPending_ = outputLeft_ == 0;
  }

  shiftUp();
  phase_ = Phase::PUSH;
  nextStepMs_ = nowMs + kPushPauseMs;
  return true;
}

bool Terminal::advance(uint32_t nowMs) {
  if (separatorPending_) {
    separatorPending_ = false;
    shiftUp();
    nextStepMs_ = nowMs + kPushPauseMs;
    return true;
  }

  if (outputLeft_ > 0) {
    startOutput(nowMs);
    return false;
  }

  startCommand(nowMs);
  return true;
}

void Terminal::shiftUp() {
  for (uint8_t y = 0; y + 1 < Display::kHeight; y++) {
    rows_[y] = rows_[y + 1];
  }

  rows_[kInputRow] = 0;
}

bool Terminal::updateCaret(uint32_t nowMs) {
  if (static_cast<int32_t>(nowMs - nextCaretMs_) < 0) {
    return false;
  }

  caretOn_ = !caretOn_;
  nextCaretMs_ = nowMs + kCaretBlinkMs;
  return caretRuns();
}

bool Terminal::caretRuns() const {
  return phase_ == Phase::TYPING && command_;
}

bool Terminal::caretShown() const {
  return caretRuns() && caretOn_;
}

void Terminal::draw(Display& display) const {
  display.setBrightness(maxBrightness());
  display.clear();

  for (uint8_t y = 0; y < Display::kHeight; y++) {
    for (uint8_t x = 0; x < Display::kWidth; x++) {
      display.setPixel(x, y, rows_[y] & (1 << x));
    }
  }

  display.setPixel(kPromptColumn, kInputRow, true);

  if (caretShown()) {
    display.setPixel(column_, kInputRow, true);
  }

  display.show();
}
