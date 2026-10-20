#include "animations/Glitch.h"

#include "animations/glitchFrame.h"

#include <Arduino.h>

const char* Glitch::name() const {
  return "glitch";
}

void Glitch::begin(Display& display) {
  startMs_ = millis();
  burst_ = kNoBurstYet;
  finished_ = false;
  held_ = false;
  step(display, 0);
}

void Glitch::tick(Display& display, uint32_t nowMs) {
  const uint32_t elapsed = nowMs - startMs_;

  if (elapsed >= kDurationMs) {
    finished_ = true;
    hold(display);
    return;
  }

  step(display, elapsed);
}

bool Glitch::finished() const {
  return finished_;
}

void Glitch::applyBrightness(Display& display, uint32_t fadeMs) {
  if (held_) {
    display.setBrightness(maxBrightness() / 2, fadeMs);
    return;
  }

  // a running burst keeps kGlitchBrightness, independent of the ceiling
  if (burst_ == 0) {
    display.setBrightness(maxBrightness(), fadeMs);
  }
}

uint8_t Glitch::burstAt(uint32_t elapsed) {
  for (uint8_t i = 0; i < kBurstCount; i++) {
    if (elapsed >= kBurstStartMs[i] && elapsed < kBurstEndMs[i]) {
      return static_cast<uint8_t>(i + 1);
    }
  }

  return 0;
}

void Glitch::step(Display& display, uint32_t elapsed) {
  const uint8_t burst = burstAt(elapsed);

  if (burst == burst_) {
    return;
  }

  burst_ = burst;

  if (burst == 0) {
    draw(display, false);
    return;
  }

  uint8_t base[Display::kHeight];
  xRows(base);
  generateGlitchRows(base, glitchRows_);
  draw(display, true);
}

void Glitch::hold(Display& display) {
  if (held_) {
    return;
  }

  held_ = true;
  drawPlain(display, maxBrightness() / 2);
}

void Glitch::draw(Display& display, bool glitched) const {
  if (glitched) {
    drawRows(display, kGlitchBrightness, glitchRows_);
    return;
  }

  drawPlain(display, maxBrightness());
}

void Glitch::drawPlain(Display& display, uint8_t brightness) const {
  uint8_t rows[Display::kHeight];
  xRows(rows);
  drawRows(display, brightness, rows);
}
