#pragma once

#include <cstdint>

class Display;

void simOnShow(const Display& display);

/** Host stub of the matrix, stores the frame buffer and clips like the real one. */
class Display {
public:
  static constexpr uint8_t kWidth = 8;
  static constexpr uint8_t kCanvasWidth = 8;
  static constexpr uint8_t kHeight = 8;
  static constexpr uint8_t kMaxBrightness = 15;

  uint8_t rows[kHeight] = {0};
  uint8_t brightness = 8;

  void clear() {
    for (uint8_t y = 0; y < kHeight; y++) {
      rows[y] = 0;
    }
  }

  void setPixel(int8_t x, int8_t y, bool on) {
    if (x < 0 || x >= static_cast<int8_t>(kWidth) || y < 0 || y >= static_cast<int8_t>(kHeight)) {
      return;
    }

    if (on) {
      rows[y] = static_cast<uint8_t>(rows[y] | (1u << x));
      return;
    }

    rows[y] = static_cast<uint8_t>(rows[y] & ~(1u << x));
  }

  void clearCanvas() {
    clear();
  }

  void setCanvasPixel(int8_t x, int8_t y, bool on) {
    setPixel(x, y, on);
  }

  void show() {
    simOnShow(*this);
  }

  void setBrightness(uint8_t level) {
    brightness = level > kMaxBrightness ? kMaxBrightness : level;
    simOnShow(*this);
  }

  void setBrightnessNow(uint8_t level) {
    setBrightness(level);
  }

  void tick(uint32_t) {
  }
};
