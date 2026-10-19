#include "animations/Pipes.h"

#include <Arduino.h>

const char* Pipes::name() const {
  return "pipes";
}

void Pipes::begin(Display& display) {
  for (uint8_t& row : rows_) {
    row = 0;
  }

  startBlink_ = 0;
  direction_ = 0;
  crossedLast_ = false;
  startPipe(millis());
  draw(display);
}

void Pipes::tick(Display& display, uint32_t nowMs) {
  if (static_cast<int32_t>(nowMs - nextStepMs_) < 0) {
    return;
  }

  switch (phase_) {
    case Phase::START_BLINK:
      blinkStart(nowMs);
      break;
    case Phase::GROWING:
      growStep(nowMs);
      break;
    case Phase::FULL_HOLD:
      clearGrid(nowMs);
      break;
    case Phase::PAUSE:
    default:
      startPipe(nowMs);
      break;
  }

  draw(display);
}

int8_t Pipes::stepX(uint8_t direction) {
  static constexpr int8_t kSteps[kDirectionCount] = {0, 1, 0, -1};
  return kSteps[direction % kDirectionCount];
}

int8_t Pipes::stepY(uint8_t direction) {
  static constexpr int8_t kSteps[kDirectionCount] = {-1, 0, 1, 0};
  return kSteps[direction % kDirectionCount];
}

bool Pipes::inGrid(int8_t x, int8_t y) {
  return x >= 0 && y >= 0 && x < static_cast<int8_t>(Display::kWidth) &&
         y < static_cast<int8_t>(Display::kHeight);
}

bool Pipes::isLit(int8_t x, int8_t y) const {
  if (!inGrid(x, y)) {
    return false;
  }

  return (rows_[y] & (1 << x)) != 0;
}

bool Pipes::isDark(int8_t x, int8_t y) const {
  return inGrid(x, y) && !isLit(x, y);
}

bool Pipes::hasSpacing(int8_t x, int8_t y, int8_t allowX, int8_t allowY) const {
  for (uint8_t direction = 0; direction < kDirectionCount; direction++) {
    const int8_t nextX = static_cast<int8_t>(x + stepX(direction));
    const int8_t nextY = static_cast<int8_t>(y + stepY(direction));

    if (!isLit(nextX, nextY)) {
      continue;
    }

    if (nextX != allowX || nextY != allowY) {
      return false;
    }
  }

  return true;
}

bool Pipes::hasSideRoom(int8_t x, int8_t y, uint8_t direction) const {
  const uint8_t side = static_cast<uint8_t>((direction + 1) % kDirectionCount);
  const int8_t oneX = static_cast<int8_t>(x + stepX(side));
  const int8_t oneY = static_cast<int8_t>(y + stepY(side));
  const int8_t otherX = static_cast<int8_t>(x - stepX(side));
  const int8_t otherY = static_cast<int8_t>(y - stepY(side));
  return !isLit(oneX, oneY) && !isLit(otherX, otherY);
}

bool Pipes::canStep(int8_t x, int8_t y, uint8_t direction) const {
  const int8_t nextX = static_cast<int8_t>(x + stepX(direction));
  const int8_t nextY = static_cast<int8_t>(y + stepY(direction));

  if (!isDark(nextX, nextY)) {
    return false;
  }

  if (!hasSideRoom(nextX, nextY, direction)) {
    return false;
  }

  const int8_t aheadX = static_cast<int8_t>(nextX + stepX(direction));
  const int8_t aheadY = static_cast<int8_t>(nextY + stepY(direction));

  if (!isLit(aheadX, aheadY)) {
    return true;
  }

  return canCross(nextX, nextY, direction);
}

bool Pipes::canCross(int8_t x, int8_t y, uint8_t direction) const {
  const int8_t crossX = static_cast<int8_t>(x + stepX(direction));
  const int8_t crossY = static_cast<int8_t>(y + stepY(direction));

  if (!isLit(crossX, crossY)) {
    return false;
  }

  const uint8_t side = static_cast<uint8_t>((direction + 1) % kDirectionCount);
  const int8_t oneX = static_cast<int8_t>(crossX + stepX(side));
  const int8_t oneY = static_cast<int8_t>(crossY + stepY(side));
  const int8_t otherX = static_cast<int8_t>(crossX - stepX(side));
  const int8_t otherY = static_cast<int8_t>(crossY - stepY(side));
  const bool oneLit = isLit(oneX, oneY);
  const bool otherLit = isLit(otherX, otherY);
  const bool oneRuns = oneLit || !inGrid(oneX, oneY);
  const bool otherRuns = otherLit || !inGrid(otherX, otherY);

  if (!oneRuns || !otherRuns) {
    return false;
  }

  if (!oneLit && !otherLit) {
    return false;
  }

  const int8_t farX = static_cast<int8_t>(crossX + stepX(direction));
  const int8_t farY = static_cast<int8_t>(crossY + stepY(direction));

  if (!isDark(farX, farY)) {
    return false;
  }

  return hasSpacing(farX, farY, crossX, crossY);
}

bool Pipes::canStartAt(uint8_t x, uint8_t y) const {
  const int8_t startX = static_cast<int8_t>(x);
  const int8_t startY = static_cast<int8_t>(y);

  if (!isDark(startX, startY)) {
    return false;
  }

  if (!hasSpacing(startX, startY, -1, -1)) {
    return false;
  }

  for (uint8_t direction = 0; direction < kDirectionCount; direction++) {
    if (canStep(startX, startY, direction)) {
      return true;
    }
  }

  return false;
}

bool Pipes::pickStart() {
  uint8_t seen = 0;
  uint8_t pickedX = 0;
  uint8_t pickedY = 0;

  for (uint8_t y = 0; y < Display::kHeight; y++) {
    for (uint8_t x = 0; x < Display::kWidth; x++) {
      if (!canStartAt(x, y)) {
        continue;
      }

      seen++;

      // reservoir sampling, every candidate gets the same chance in one pass
      if (random(seen) == 0) {
        pickedX = x;
        pickedY = y;
      }
    }
  }

  if (seen == 0) {
    return false;
  }

  headX_ = pickedX;
  headY_ = pickedY;
  rows_[headY_] = static_cast<uint8_t>(rows_[headY_] | (1 << headX_));
  return true;
}

void Pipes::startPipe(uint32_t nowMs) {
  crossedLast_ = false;

  if (pickStart()) {
    startBlink_ = kStartBlinkSteps;
    phase_ = Phase::START_BLINK;
    nextStepMs_ = nowMs + kStartBlinkMs;
    return;
  }

  phase_ = Phase::FULL_HOLD;
  nextStepMs_ = nowMs + kFullHoldMs;
}

void Pipes::blinkStart(uint32_t nowMs) {
  startBlink_--;

  if (startBlink_ > 0) {
    nextStepMs_ = nowMs + kStartBlinkMs;
    return;
  }

  uint8_t exits[kDirectionCount] = {};
  uint8_t count = 0;

  for (uint8_t direction = 0; direction < kDirectionCount; direction++) {
    if (canStep(static_cast<int8_t>(headX_), static_cast<int8_t>(headY_), direction)) {
      exits[count] = direction;
      count++;
    }
  }

  direction_ = exits[random(count)];
  phase_ = Phase::GROWING;
  nextStepMs_ = nowMs + kStepMs;
}

void Pipes::growStep(uint32_t nowMs) {
  const int8_t aheadX = static_cast<int8_t>(headX_ + stepX(direction_));
  const int8_t aheadY = static_cast<int8_t>(headY_ + stepY(direction_));

  if (isLit(aheadX, aheadY)) {
    // a pipe only ever stops in front of one it may cross, turning away here would leave the two
    // touching side by side
    if (!crossedLast_ &&
        canCross(static_cast<int8_t>(headX_), static_cast<int8_t>(headY_), direction_)) {
      crossOver();
      nextStepMs_ = nowMs + kStepMs;
      return;
    }

    endPipe(nowMs);
    return;
  }

  const bool straightAllowed =
      canStep(static_cast<int8_t>(headX_), static_cast<int8_t>(headY_), direction_);

  if (!straightAllowed || random(100) >= kStraightPercent) {
    if (turnOrEnd(nowMs, straightAllowed)) {
      nextStepMs_ = nowMs + kStepMs;
      return;
    }

    if (!straightAllowed) {
      return;
    }
  }

  advance();
  nextStepMs_ = nowMs + kStepMs;
}

bool Pipes::turnOrEnd(uint32_t nowMs, bool straightAllowed) {
  const uint8_t left = static_cast<uint8_t>((direction_ + kDirectionCount - 1) % kDirectionCount);
  const uint8_t right = static_cast<uint8_t>((direction_ + 1) % kDirectionCount);
  uint8_t turns[2] = {};
  uint8_t count = 0;

  if (canStep(static_cast<int8_t>(headX_), static_cast<int8_t>(headY_), left)) {
    turns[count] = left;
    count++;
  }

  if (canStep(static_cast<int8_t>(headX_), static_cast<int8_t>(headY_), right)) {
    turns[count] = right;
    count++;
  }

  if (count > 0) {
    direction_ = turns[random(count)];
    advance();
    return true;
  }

  if (!straightAllowed) {
    endPipe(nowMs);
  }

  return false;
}

void Pipes::advance() {
  headX_ = static_cast<uint8_t>(headX_ + stepX(direction_));
  headY_ = static_cast<uint8_t>(headY_ + stepY(direction_));
  rows_[headY_] = static_cast<uint8_t>(rows_[headY_] | (1 << headX_));
  crossedLast_ = false;
}

void Pipes::crossOver() {
  headX_ = static_cast<uint8_t>(headX_ + 2 * stepX(direction_));
  headY_ = static_cast<uint8_t>(headY_ + 2 * stepY(direction_));
  rows_[headY_] = static_cast<uint8_t>(rows_[headY_] | (1 << headX_));
  crossedLast_ = true;
}

void Pipes::endPipe(uint32_t nowMs) {
  phase_ = Phase::PAUSE;
  nextStepMs_ = nowMs + kPipePauseMs;
}

void Pipes::clearGrid(uint32_t nowMs) {
  for (uint8_t& row : rows_) {
    row = 0;
  }

  startPipe(nowMs);
}

void Pipes::draw(Display& display) const {
  display.setBrightness(maxBrightness());
  display.clear();

  for (uint8_t y = 0; y < Display::kHeight; y++) {
    for (uint8_t x = 0; x < Display::kWidth; x++) {
      bool lit = (rows_[y] & (1 << x)) != 0;

      if (phase_ == Phase::START_BLINK && x == headX_ && y == headY_) {
        lit = startBlink_ % 2 == 0;
      }

      display.setPixel(static_cast<int8_t>(x), static_cast<int8_t>(y), lit);
    }
  }

  display.show();
}
