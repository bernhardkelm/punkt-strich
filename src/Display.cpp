#include "Display.h"

#include <Arduino.h>
#include <Wire.h>

bool Display::begin(uint8_t sda, uint8_t scl) {
  failedAddress_ = 0;

  if (!wireStarted_) {
    wireStarted_ = Wire.begin(sda, scl);
  }

  if (!wireStarted_) {
    failedAddress_ = Board::kMatrixAddresses[0];
    return false;
  }

  for (uint8_t m = 0; m < Board::kMatrixCount; ++m) {
    if (!matrices_[m].begin(Board::kMatrixAddresses[m], &Wire)) {
      failedAddress_ = Board::kMatrixAddresses[m];
      return false;
    }
  }

  Wire.setClock(kI2cHz);

  for (Adafruit_8x8matrix& matrix : matrices_) {
    matrix.setRotation(0);
  }

  brightness_.reset();
  writtenLevel_ = kLevelUnset;
  resetLayers();
  clear();
  show();
  return true;
}

uint8_t Display::failedAddress() const {
  return failedAddress_;
}

void Display::show() {
  const Frame out = compose();

  for (uint8_t m = 0; m < Board::kMatrixCount; ++m) {
    const uint8_t left = static_cast<uint8_t>(m * kMatrixWidth);

    for (uint8_t y = 0; y < kHeight; ++y) {
      for (uint8_t x = 0; x < kMatrixWidth; ++x) {
        const bool on = ((out.rows[y] >> (left + x)) & 0x01) != 0;
        matrices_[m].drawPixel(x, y, on ? LED_ON : LED_OFF);
      }
    }

    matrices_[m].writeDisplay();
  }

  dirty_ = false;
}

void Display::setBrightness(uint8_t level, uint32_t durationMs) {
  requested_ = level > kMaxBrightness ? kMaxBrightness : level;
  brightness_.rampTo(effectiveBrightness(), durationMs, millis());
}

void Display::setBrightnessNow(uint8_t level) {
  requested_ = level > kMaxBrightness ? kMaxBrightness : level;
  brightness_.rampTo(effectiveBrightness(), 0, millis());
}

void Display::setBrightnessFloor(uint8_t level) {
  const uint8_t clamped = level > kMaxBrightness ? kMaxBrightness : level;

  if (clamped == floor_) {
    return;
  }

  floor_ = clamped;
  brightness_.rampTo(effectiveBrightness(), kFadeMs, millis());
}

uint8_t Display::brightness() const {
  if (writtenLevel_ == kLevelUnset) {
    return requested_;
  }

  return writtenLevel_;
}

void Display::tick(uint32_t nowMs) {
  uint8_t level = 0;

  if (brightness_.tick(nowMs, level)) {
    for (Adafruit_8x8matrix& matrix : matrices_) {
      matrix.setBrightness(level);
    }

    writtenLevel_ = level;
  }

  if (dirty_) {
    show();
  }
}

void Display::standby() {
  dropOverlayAndReveal();

  for (uint8_t m = 0; m < Board::kMatrixCount; ++m) {
    matrices_[m].clear();
    matrices_[m].writeDisplay();
    matrices_[m].setDisplayState(false);
    Wire.beginTransmission(Board::kMatrixAddresses[m]);
    Wire.write(kOscillatorOff);
    Wire.endTransmission();
  }
}

uint8_t Display::effectiveBrightness() const {
  return requested_ > floor_ ? requested_ : floor_;
}
