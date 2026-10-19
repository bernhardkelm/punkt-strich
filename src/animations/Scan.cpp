#include "animations/Scan.h"

#include <Arduino.h>

const char* Scan::name() const {
  return "scan";
}

void Scan::begin(Display& display) {
  startPage(millis());
  draw(display);
}

void Scan::tick(Display& display, uint32_t nowMs) {
  if (static_cast<int32_t>(nowMs - nextStepMs_) < 0) {
    return;
  }

  switch (phase_) {
    case Phase::READING:
      readStep(nowMs);
      draw(display);
      break;
    case Phase::PAUSE:
      advanceRow(nowMs);
      draw(display);
      break;
    case Phase::HOLD:
    default:
      phase_ = Phase::SCROLL;
      scrollStep(nowMs);
      draw(display);
      break;
  }
}

uint8_t Scan::rowBits(const LinePlan& plan) {
  uint8_t bits = 0;

  for (uint8_t i = 0; i < plan.length; i++) {
    const uint8_t column = static_cast<uint8_t>(plan.indent + i);

    if (static_cast<int8_t>(column) == plan.gapColumn) {
      continue;
    }

    bits = static_cast<uint8_t>(bits | (1 << column));
  }

  return bits;
}

uint8_t Scan::firstLitColumn(uint8_t bits) {
  for (uint8_t x = 0; x < Display::kWidth; x++) {
    if (bits & (1 << x)) {
      return x;
    }
  }

  return 0;
}

uint8_t Scan::lastLitColumn(uint8_t bits) {
  for (uint8_t x = Display::kWidth; x > 0; x--) {
    if (bits & (1 << (x - 1))) {
      return static_cast<uint8_t>(x - 1);
    }
  }

  return 0;
}

uint8_t Scan::nextLitRow(uint8_t from) const {
  for (uint8_t y = from; y < Display::kHeight; y++) {
    if (rows_[y] != 0) {
      return y;
    }
  }

  return Display::kHeight;
}

bool Scan::pageEmpty() const {
  return nextLitRow(0) >= Display::kHeight;
}

void Scan::planPage() {
  LinePlan previous;
  uint8_t indent = 0;

  for (uint8_t y = 0; y < Display::kHeight; y++) {
    const LinePlan plan = planLine(previous, indent, y == 0);
    rows_[y] = rowBits(plan);
    indent = nextIndent(plan);
    previous = plan;
  }
}

void Scan::startPage(uint32_t nowMs) {
  planPage();
  row_ = nextLitRow(0);
  startRow(nowMs);
}

void Scan::startRow(uint32_t nowMs) {
  if (row_ >= Display::kHeight) {
    phase_ = Phase::HOLD;
    nextStepMs_ = nowMs + kPageHoldMs;
    return;
  }

  column_ = firstLitColumn(rows_[row_]);
  endColumn_ = lastLitColumn(rows_[row_]);
  phase_ = Phase::READING;
  nextStepMs_ = nowMs + kCursorStepMs;
}

void Scan::readStep(uint32_t nowMs) {
  if (column_ >= endColumn_) {
    endRow(nowMs);
    return;
  }

  column_++;
  nextStepMs_ = nowMs + kCursorStepMs;
}

void Scan::endRow(uint32_t nowMs) {
  if (nextLitRow(static_cast<uint8_t>(row_ + 1)) >= Display::kHeight) {
    phase_ = Phase::HOLD;
    nextStepMs_ = nowMs + kPageHoldMs;
    return;
  }

  phase_ = Phase::PAUSE;
  nextStepMs_ = nowMs + kLinePauseMs;
}

void Scan::advanceRow(uint32_t nowMs) {
  row_ = nextLitRow(static_cast<uint8_t>(row_ + 1));
  startRow(nowMs);
}

void Scan::scrollStep(uint32_t nowMs) {
  if (pageEmpty()) {
    startPage(nowMs);
    return;
  }

  for (uint8_t y = 0; y + 1 < Display::kHeight; y++) {
    rows_[y] = rows_[y + 1];
  }

  rows_[Display::kHeight - 1] = 0;
  nextStepMs_ = nowMs + kScrollStepMs;
}

void Scan::draw(Display& display) const {
  display.setBrightness(maxBrightness());
  display.clear();

  for (uint8_t y = 0; y < Display::kHeight; y++) {
    for (uint8_t x = 0; x < Display::kWidth; x++) {
      const bool lit = rows_[y] & (1 << x);
      const bool cursor = phase_ == Phase::READING && y == row_ && x == column_;
      display.setPixel(x, y, cursor ? !lit : lit);
    }
  }

  display.show();
}
