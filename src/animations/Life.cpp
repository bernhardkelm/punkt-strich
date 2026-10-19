#include "animations/Life.h"

#include <Arduino.h>
#include <cstring>

const char* Life::name() const {
  return "life";
}

void Life::begin(Display& display) {
  seed();
  nextStepMs_ = millis() + kStepMs;
  draw(display);
}

void Life::tick(Display& display, uint32_t nowMs) {
  if (static_cast<int32_t>(nowMs - nextStepMs_) < 0) {
    return;
  }

  nextStepMs_ = nowMs + kStepMs;
  step();

  if (generation_ >= kMaxGenerations || isDead() || isStuck()) {
    seed();
  }

  draw(display);
}

void Life::seed() {
  for (uint8_t& row : rows_) {
    row = 0;

    for (uint8_t x = 0; x < Display::kWidth; x++) {
      if (random(100) < kSeedDensityPercent) {
        row = static_cast<uint8_t>(row | (1 << x));
      }
    }
  }

  std::memset(previous_, 0, sizeof(previous_));
  std::memset(beforePrevious_, 0, sizeof(beforePrevious_));
  generation_ = 0;
}

void Life::step() {
  Grid next = {};
  evolve(rows_, next);
  std::memcpy(beforePrevious_, previous_, sizeof(beforePrevious_));
  std::memcpy(previous_, rows_, sizeof(previous_));
  std::memcpy(rows_, next, sizeof(rows_));
  generation_++;
}

bool Life::isStuck() const {
  return std::memcmp(rows_, previous_, sizeof(rows_)) == 0 ||
         std::memcmp(rows_, beforePrevious_, sizeof(rows_)) == 0;
}

bool Life::isDead() const {
  for (uint8_t row : rows_) {
    if (row != 0) {
      return false;
    }
  }

  return true;
}

void Life::evolve(const Grid& from, Grid& to) {
  for (uint8_t y = 0; y < Display::kHeight; y++) {
    to[y] = 0;

    for (uint8_t x = 0; x < Display::kWidth; x++) {
      const uint8_t n = neighbours(from, x, y);
      const bool alive = from[y] & (1 << x);

      if (n == 3 || (alive && n == 2)) {
        to[y] = static_cast<uint8_t>(to[y] | (1 << x));
      }
    }
  }
}

uint8_t Life::neighbours(const Grid& grid, uint8_t x, uint8_t y) {
  uint8_t count = 0;

  for (int8_t dy = -1; dy <= 1; dy++) {
    for (int8_t dx = -1; dx <= 1; dx++) {
      if (dx == 0 && dy == 0) {
        continue;
      }

      const uint8_t nx = (x + dx + Display::kWidth) % Display::kWidth;
      const uint8_t ny = (y + dy + Display::kHeight) % Display::kHeight;

      if (grid[ny] & (1 << nx)) {
        count++;
      }
    }
  }

  return count;
}

void Life::draw(Display& display) const {
  display.setBrightness(maxBrightness());
  display.clear();

  for (uint8_t y = 0; y < Display::kHeight; y++) {
    for (uint8_t x = 0; x < Display::kWidth; x++) {
      display.setPixel(x, y, rows_[y] & (1 << x));
    }
  }

  display.show();
}
