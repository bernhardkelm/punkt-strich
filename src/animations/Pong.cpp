#include "animations/Pong.h"

#include <Arduino.h>

const char* Pong::name() const {
  return "pong";
}

void Pong::begin(Display& display) {
  leftTop_ = kRestTop;
  rightTop_ = kRestTop;
  loserColumn_ = random(2) == 0 ? kLeftColumn : kRightColumn;
  serve(millis());
  draw(display);
}

void Pong::tick(Display& display, uint32_t nowMs) {
  if (static_cast<int32_t>(nowMs - nextStepMs_) < 0) {
    return;
  }

  if (phase_ == Phase::SCORE) {
    flashScore(nowMs);
    draw(display);
    return;
  }

  movePaddles();
  moveBall(nowMs);
  draw(display);
}

int8_t Pong::clampTop(int8_t top) {
  if (top < 0) {
    return 0;
  }

  if (top > kMaxTop) {
    return kMaxTop;
  }

  return top;
}

bool Pong::covers(int8_t top, int8_t y) {
  return y >= top && y < top + kPaddleHeight;
}

int8_t Pong::movePaddle(int8_t top, int8_t targetTop, bool tracking) {
  if (tracking && random(100) < kHesitatePercent) {
    return top;
  }

  if (top < targetTop) {
    return static_cast<int8_t>(top + 1);
  }

  if (top > targetTop) {
    return static_cast<int8_t>(top - 1);
  }

  return top;
}

void Pong::movePaddles() {
  const bool comesLeft = dx_ < 0;
  const int8_t aimed = clampTop(static_cast<int8_t>(ballY_ - 1));
  leftTop_ = movePaddle(leftTop_, comesLeft ? aimed : kRestTop, comesLeft);
  rightTop_ = movePaddle(rightTop_, comesLeft ? kRestTop : aimed, !comesLeft);
}

void Pong::restPaddles() {
  leftTop_ = movePaddle(leftTop_, kRestTop, false);
  rightTop_ = movePaddle(rightTop_, kRestTop, false);
}

void Pong::moveBall(uint32_t nowMs) {
  if (ballY_ + dy_ < 0 || ballY_ + dy_ >= static_cast<int8_t>(Display::kHeight)) {
    dy_ = static_cast<int8_t>(-dy_);
  }

  ballY_ = static_cast<int8_t>(ballY_ + dy_);
  const int8_t nextX = static_cast<int8_t>(ballX_ + dx_);

  if (nextX == kLeftColumn || nextX == kRightColumn) {
    const int8_t top = nextX == kLeftColumn ? leftTop_ : rightTop_;

    if (!covers(top, ballY_)) {
      score(nextX, nowMs);
      return;
    }

    dx_ = static_cast<int8_t>(-dx_);
    nextStepMs_ = nowMs + kStepMs;
    return;
  }

  ballX_ = nextX;
  nextStepMs_ = nowMs + kStepMs;
}

void Pong::score(int8_t column, uint32_t nowMs) {
  loserColumn_ = column;
  flashes_ = kScoreFlashes;
  phase_ = Phase::SCORE;
  nextStepMs_ = nowMs + kScoreFlashMs;
}

void Pong::flashScore(uint32_t nowMs) {
  restPaddles();
  flashes_--;

  if (flashes_ > 0) {
    nextStepMs_ = nowMs + kScoreFlashMs;
    return;
  }

  serve(nowMs);
}

void Pong::serve(uint32_t nowMs) {
  const bool towardsLeft = loserColumn_ == kLeftColumn;
  dx_ = towardsLeft ? -1 : 1;
  dy_ = random(2) == 0 ? -1 : 1;
  ballX_ = towardsLeft ? static_cast<int8_t>(kRightColumn - 3) : 3;
  ballY_ = static_cast<int8_t>(Display::kHeight / 2);
  phase_ = Phase::PLAY;
  nextStepMs_ = nowMs + kServePauseMs;
}

void Pong::drawPaddle(Display& display, int8_t column, int8_t top) {
  for (int8_t row = 0; row < kPaddleHeight; row++) {
    display.setPixel(column, static_cast<int8_t>(top + row), true);
  }
}

void Pong::draw(Display& display) const {
  display.setBrightness(maxBrightness());
  display.clear();
  drawPaddle(display, kLeftColumn, leftTop_);
  drawPaddle(display, kRightColumn, rightTop_);

  if (phase_ == Phase::SCORE) {
    if (flashes_ % 2 == 0) {
      for (int8_t row = 0; row < static_cast<int8_t>(Display::kHeight); row++) {
        display.setPixel(loserColumn_, row, true);
      }
    }

    display.show();
    return;
  }

  display.setPixel(ballX_, ballY_, true);
  display.show();
}
