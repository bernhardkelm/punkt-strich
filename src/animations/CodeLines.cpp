#include "animations/CodeLines.h"

#include <Arduino.h>

const char* CodeLines::name() const {
  return "code";
}

void CodeLines::begin(Display& display) {
  restart(millis());
  draw(display);
}

void CodeLines::tick(Display& display, uint32_t nowMs) {
  if (static_cast<int32_t>(nowMs - nextStepMs_) < 0) {
    return;
  }

  switch (phase_) {
    case Phase::TYPING:
      typeStep(nowMs);
      draw(display);
      break;
    case Phase::PAUSE:
      advanceRow(nowMs);
      break;
    case Phase::HOLD:
    default:
      restart(nowMs);
      draw(display);
      break;
  }
}

void CodeLines::startRow(uint32_t nowMs) {
  plan_ = planLine(plan_, indent_, row_ == 0);

  if (plan_.blank) {
    endRow(nowMs);
    return;
  }

  indent_ = nextIndent(plan_);
  column_ = plan_.indent;
  endColumn_ = static_cast<uint8_t>(plan_.indent + plan_.length - 1);
  phase_ = Phase::TYPING;
  nextStepMs_ = nowMs + kTypeStepMs;
}

void CodeLines::typeStep(uint32_t nowMs) {
  if (static_cast<int8_t>(column_) != plan_.gapColumn) {
    rows_[row_] = static_cast<uint8_t>(rows_[row_] | (1 << column_));
  }

  if (column_ >= endColumn_) {
    endRow(nowMs);
    return;
  }

  column_++;
  nextStepMs_ = nowMs + kTypeStepMs;
}

void CodeLines::endRow(uint32_t nowMs) {
  if (row_ + 1 >= Display::kHeight) {
    phase_ = Phase::HOLD;
    nextStepMs_ = nowMs + kRestartHoldMs;
    return;
  }

  phase_ = Phase::PAUSE;
  nextStepMs_ = nowMs + kLinePauseMs;
}

void CodeLines::advanceRow(uint32_t nowMs) {
  row_++;
  startRow(nowMs);
}

void CodeLines::restart(uint32_t nowMs) {
  for (uint8_t& row : rows_) {
    row = 0;
  }

  row_ = 0;
  indent_ = 0;
  plan_ = LinePlan();
  startRow(nowMs);
}

void CodeLines::draw(Display& display) const {
  display.setBrightness(maxBrightness());
  display.clear();

  for (uint8_t y = 0; y < Display::kHeight; y++) {
    for (uint8_t x = 0; x < Display::kWidth; x++) {
      display.setPixel(x, y, rows_[y] & (1 << x));
    }
  }

  display.show();
}
