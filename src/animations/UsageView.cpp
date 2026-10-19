#include "animations/UsageView.h"

#include "animations/blink.h"
#include "animations/bottomFill.h"

#include <Arduino.h>

const char* UsageView::name() const {
  return "usage";
}

void UsageView::setPercent(uint8_t percent) {
  percent_ = percent > 100 ? 100 : percent;
}

void UsageView::begin(Display& display) {
  startMs_ = millis();

  const uint8_t percent = percent_;
  pixels_ = pixelsFor(percent);
  lit_ = topRowLit(0, percent);
  brightness_ = brightnessAt(0, percent);
  display.setBrightnessNow(brightness_);
  draw(display, pixels_, lit_);
}

void UsageView::tick(Display& display, uint32_t nowMs) {
  const uint32_t phase = nowMs - startMs_;
  const uint8_t percent = percent_;
  const uint8_t pixels = pixelsFor(percent);
  const bool lit = topRowLit(phase, percent);
  const uint8_t brightness = brightnessAt(phase, percent);

  if (brightness != brightness_) {
    brightness_ = brightness;
    display.setBrightnessNow(brightness_);
  }

  if (pixels == pixels_ && lit == lit_) {
    return;
  }

  pixels_ = pixels;
  lit_ = lit;
  draw(display, pixels_, lit_);
}

uint8_t UsageView::pixelsFor(uint8_t percent) {
  const uint16_t capped = percent > 100 ? 100 : percent;
  return static_cast<uint8_t>((capped * kPixelCount + 50) / 100);
}

bool UsageView::topRowLit(uint32_t phase, uint8_t percent) {
  if (percent <= kBlinkPercent || percent > kPulsePercent) {
    return true;
  }

  return blinkLitFirst(phase, kBlinkPeriodMs, kBlinkOnMs);
}

uint8_t UsageView::dim() const {
  const uint8_t level = maxBrightness() / kDimDivisor;
  return level > 0 ? level : 1;
}

uint8_t UsageView::brightnessAt(uint32_t phase, uint8_t percent) const {
  if (percent == 0) {
    return dim();
  }

  if (percent <= kPulsePercent) {
    return maxBrightness();
  }

  const uint8_t low = dim();
  const uint8_t span = maxBrightness() > low ? static_cast<uint8_t>(maxBrightness() - low) : 0;
  const uint32_t half = kPulsePeriodMs / 2;
  const uint32_t stepped = (phase / kPulseStepMs * kPulseStepMs) % kPulsePeriodMs;
  const uint32_t rising = stepped < half ? stepped : kPulsePeriodMs - stepped;
  return static_cast<uint8_t>(low + (span * rising + half / 2) / half);
}

void UsageView::draw(Display& display, uint8_t pixels, bool lit) {
  display.clear();

  if (pixels == 0) {
    display.setPixel(0, Display::kHeight - 1, true);
    display.show();
    return;
  }

  const uint8_t topRow = static_cast<uint8_t>((pixels - 1) / Display::kWidth);

  for (uint8_t n = 0; n < pixels; n++) {
    const uint8_t row = static_cast<uint8_t>(n / Display::kWidth);

    if (row == topRow && !lit) {
      continue;
    }

    setBottomFillPixel(display, n, true);
  }

  display.show();
}
