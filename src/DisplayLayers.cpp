#include "Display.h"

#include <cstring>

namespace {

/**
 * Returns whether two frames hold different pixels.
 * @param {const Frame&} a - first frame
 * @param {const Frame&} b - second frame
 * @returns {bool} - true when any row differs
 */
bool differs(const Frame& a, const Frame& b) {
  return std::memcmp(a.rows, b.rows, sizeof(a.rows)) != 0;
}

}

void Display::clear() {
  const uint16_t mask = static_cast<uint16_t>(kViewportBits << viewportX_);

  for (uint8_t y = 0; y < kHeight; ++y) {
    base_.rows[y] = static_cast<uint16_t>(base_.rows[y] & ~mask);
  }
}

void Display::clearCanvas() {
  base_ = Frame();
}

void Display::setPixel(int8_t x, int8_t y, bool on) {
  if (x < 0 || y < 0 || x >= kWidth || y >= kHeight) {
    return;
  }

  setCanvasPixel(static_cast<int8_t>(x + viewportX_), y, on);
}

void Display::setCanvasPixel(int8_t x, int8_t y, bool on) {
  if (x < 0 || y < 0 || x >= kCanvasWidth || y >= kHeight) {
    return;
  }

  const uint16_t bit = static_cast<uint16_t>(1 << x);

  if (on) {
    base_.rows[y] = static_cast<uint16_t>(base_.rows[y] | bit);
  } else {
    base_.rows[y] = static_cast<uint16_t>(base_.rows[y] & ~bit);
  }
}

void Display::setViewportX(uint8_t x) {
  viewportX_ = x > kMaxViewportX ? kMaxViewportX : x;
}

void Display::setOverlay(const Frame& on, const Frame& off) {
  if (!differs(overlayOn_, on) && !differs(overlayOff_, off)) {
    return;
  }

  overlayOn_ = on;
  overlayOff_ = off;
  dirty_ = true;
}

void Display::clearOverlay() {
  const Frame empty;
  setOverlay(empty, empty);
}

void Display::freeze() {
  frozen_ = base_;
  reveal_ = Frame();
  revealing_ = true;
  freezeCount_++;
}

void Display::setReveal(const Frame& mask) {
  if (!differs(reveal_, mask)) {
    return;
  }

  reveal_ = mask;
  dirty_ = true;
}

void Display::endReveal() {
  if (!revealing_) {
    return;
  }

  revealing_ = false;
  reveal_ = Frame();
  dirty_ = true;
}

bool Display::revealing() const {
  return revealing_;
}

uint8_t Display::freezeCount() const {
  return freezeCount_;
}

Frame Display::compose() const {
  Frame out;

  for (uint8_t y = 0; y < kHeight; ++y) {
    uint16_t live = base_.rows[y];

    if (revealing_) {
      const uint16_t mask = reveal_.rows[y];
      const uint16_t held = static_cast<uint16_t>(frozen_.rows[y] & ~mask);
      live = static_cast<uint16_t>((base_.rows[y] & mask) | held);
    }

    const uint16_t kept = static_cast<uint16_t>(live & ~overlayOff_.rows[y]);
    out.rows[y] = static_cast<uint16_t>(kept | overlayOn_.rows[y]);
  }

  return out;
}

void Display::resetLayers() {
  base_ = Frame();
  frozen_ = Frame();
  dropOverlayAndReveal();
}

void Display::dropOverlayAndReveal() {
  overlayOn_ = Frame();
  overlayOff_ = Frame();
  reveal_ = Frame();
  revealing_ = false;
  dirty_ = false;
}
