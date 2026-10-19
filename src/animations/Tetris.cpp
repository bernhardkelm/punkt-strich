#include "animations/Tetris.h"

#include <Arduino.h>

const char* Tetris::name() const {
  return "tetris";
}

void Tetris::begin(Display& display) {
  resetStack();
  flashing_ = false;
  flashOn_ = false;
  spawnPiece();
  nextStepMs_ = millis() + kStepMs;
  draw(display);
}

void Tetris::tick(Display& display, uint32_t nowMs) {
  if (flashing_) {
    if (nowMs - flashStartMs_ >= kStepMs) {
      flashing_ = false;
      collapseFullRows();
      spawnPiece();
      nextStepMs_ = nowMs + kStepMs;
      draw(display);
      return;
    }

    const bool on = ((nowMs - flashStartMs_) / kFlashToggleMs) % 2 == 0;

    if (on == flashOn_) {
      return;
    }

    flashOn_ = on;
    draw(display);
    return;
  }

  if (static_cast<int32_t>(nowMs - nextStepMs_) < 0) {
    return;
  }

  nextStepMs_ = nowMs + kStepMs;

  if (!collides(pieceX_, static_cast<int8_t>(pieceY_ + 1))) {
    pieceY_++;
    draw(display);
    return;
  }

  mergePiece();

  if (hasFullRows()) {
    flashing_ = true;
    flashOn_ = true;
    flashStartMs_ = nowMs;
    draw(display);
    return;
  }

  spawnPiece();
  draw(display);
}

void Tetris::resetStack() {
  for (uint8_t& row : stack_) {
    row = 0;
  }
}

void Tetris::spawnPiece() {
  if (stack_[0] != 0) {
    resetStack();
  }

  pieceWidth_ = static_cast<uint8_t>(1 + random(2));
  pieceHeight_ = static_cast<uint8_t>(1 + random(2));
  pieceX_ = static_cast<int8_t>(random(Display::kWidth - pieceWidth_ + 1));
  pieceY_ = 0;

  if (collides(pieceX_, pieceY_)) {
    resetStack();
  }
}

void Tetris::mergePiece() {
  const uint8_t mask = pieceMask(pieceX_);

  for (int8_t row = pieceY_; row < pieceY_ + pieceHeight_; row++) {
    stack_[row] |= mask;
  }
}

void Tetris::collapseFullRows() {
  int8_t write = Display::kHeight - 1;

  for (int8_t y = Display::kHeight - 1; y >= 0; y--) {
    if (stack_[y] == kFullRow) {
      continue;
    }

    stack_[write] = stack_[y];
    write--;
  }

  while (write >= 0) {
    stack_[write] = 0;
    write--;
  }
}

uint8_t Tetris::pieceMask(int8_t x) const {
  uint8_t mask = 0;

  for (uint8_t i = 0; i < pieceWidth_; i++) {
    mask = static_cast<uint8_t>(mask | (1 << (x + i)));
  }

  return mask;
}

bool Tetris::collides(int8_t x, int8_t y) const {
  if (y + pieceHeight_ > Display::kHeight) {
    return true;
  }

  const uint8_t mask = pieceMask(x);

  for (int8_t row = y; row < y + pieceHeight_; row++) {
    if (stack_[row] & mask) {
      return true;
    }
  }

  return false;
}

bool Tetris::hasFullRows() const {
  for (uint8_t row : stack_) {
    if (row == kFullRow) {
      return true;
    }
  }

  return false;
}

void Tetris::draw(Display& display) const {
  display.setBrightness(maxBrightness());
  display.clear();

  for (uint8_t y = 0; y < Display::kHeight; y++) {
    const bool blank = flashing_ && stack_[y] == kFullRow && !flashOn_;

    for (uint8_t x = 0; x < Display::kWidth; x++) {
      display.setPixel(x, y, !blank && (stack_[y] & (1 << x)));
    }
  }

  if (!flashing_) {
    for (int8_t row = pieceY_; row < pieceY_ + pieceHeight_; row++) {
      for (uint8_t i = 0; i < pieceWidth_; i++) {
        display.setPixel(static_cast<int8_t>(pieceX_ + i), row, true);
      }
    }
  }

  display.show();
}
