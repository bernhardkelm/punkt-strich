#include "animations/MatrixRain.h"

#include <Arduino.h>

const char* MatrixRain::name() const {
  return "rain";
}

void MatrixRain::begin(Display& display) {
  const uint32_t nowMs = millis();

  for (Column& column : columns_) {
    column.running = false;
  }

  // one restart roll before the first frame, so a streak can already be on the matrix
  rollRestarts(nowMs);
  nextFrameMs_ = nowMs + kFrameMs;
  draw(display);
}

void MatrixRain::tick(Display& display, uint32_t nowMs) {
  if (static_cast<int32_t>(nowMs - nextFrameMs_) < 0) {
    return;
  }

  nextFrameMs_ = nowMs + kFrameMs;
  bool changed = stepColumns(nowMs);

  if (rollRestarts(nowMs)) {
    changed = true;
  }

  if (changed) {
    draw(display);
  }
}

void MatrixRain::startColumn(Column& column, uint32_t nowMs) const {
  const uint32_t jitterMs = kBaseStepMs * kStepJitterPercent / 100;
  const uint32_t stepMs = kBaseStepMs - jitterMs + static_cast<uint32_t>(random(2 * jitterMs + 1));

  column.running = true;
  column.head = 0;
  column.length =
      static_cast<uint8_t>(kMinTrailLength + random(kMaxTrailLength - kMinTrailLength + 1));
  column.stepMs = stepMs;
  column.nextStepMs = nowMs + stepMs;
}

bool MatrixRain::stepColumns(uint32_t nowMs) {
  bool changed = false;

  for (Column& column : columns_) {
    if (!column.running) {
      continue;
    }

    if (static_cast<int32_t>(nowMs - column.nextStepMs) < 0) {
      continue;
    }

    column.head++;
    column.nextStepMs = nowMs + column.stepMs;
    changed = true;

    if (column.head - column.length >= Display::kHeight) {
      column.running = false;
    }
  }

  return changed;
}

bool MatrixRain::rollRestarts(uint32_t nowMs) {
  bool changed = false;

  for (uint8_t x = 0; x < kActiveColumns; x++) {
    if (columns_[x].running) {
      continue;
    }

    if (random(100) < kRestartPercent) {
      startColumn(columns_[x], nowMs);
      changed = true;
    }
  }

  return changed;
}

void MatrixRain::draw(Display& display) const {
  display.setBrightness(maxBrightness());
  display.clear();

  for (uint8_t x = 0; x < Display::kWidth; x++) {
    const Column& column = columns_[x];

    if (!column.running) {
      continue;
    }

    for (uint8_t offset = 0; offset <= column.length; offset++) {
      const int8_t y = static_cast<int8_t>(column.head - offset);
      display.setPixel(static_cast<int8_t>(x), y, true);
    }
  }

  display.show();
}
