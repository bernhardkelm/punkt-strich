#pragma once

#include "Animation.h"

/** Four pixel comet running clockwise around the border of the viewport or of the whole canvas. */
class Orbit : public Animation {
public:
  /**
   * Creates the comet.
   * @param {bool} fullCanvas - true circles the whole canvas, false the viewport
   */
  explicit Orbit(bool fullCanvas = false);

  const char* name() const override;
  void begin(Display& display) override;
  void tick(Display& display, uint32_t nowMs) override;

private:
  static constexpr uint32_t kStepMs = 45;
  static constexpr uint8_t kCometLength = 4;

  bool fullCanvas_;
  uint8_t width_;
  uint8_t pathLength_;
  uint32_t startMs_ = 0;
  uint8_t head_ = 0;

  /**
   * Resolves a step index on the border path to coordinates.
   * @param {uint8_t} step - 0 to pathLength_ - 1, clockwise from the top left
   * @param {int8_t&} x - receives the column
   * @param {int8_t&} y - receives the row
   * @returns {void}
   */
  void positionAt(uint8_t step, int8_t& x, int8_t& y) const;

  /**
   * Draws the comet with its head at the given step, on the canvas or the viewport.
   * @param {Display&} display - matrix to draw on
   * @param {uint8_t} head - step index of the leading pixel
   * @returns {void}
   */
  void draw(Display& display, uint8_t head) const;
};
