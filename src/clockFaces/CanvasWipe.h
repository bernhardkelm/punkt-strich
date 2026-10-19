#pragma once

#include "Display.h"
#include "Frame.h"

#include <cstdint>

/**
 * Wipes a new picture in over the frozen old one, one canvas column per step, from the left or the
 * right canvas edge. The owner starts the new picture right after start(), it runs beneath the
 * frozen frame and shows column by column until the whole canvas is revealed.
 */
class CanvasWipe {
public:
  /**
   * Freezes the picture on screen, the owner starts the next one beneath it right after.
   * @param {Display&} display - display to freeze
   * @param {bool} fromLeft - true reveals the new picture from the left canvas edge
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void start(Display& display, bool fromLeft, uint32_t nowMs);

  /**
   * Reveals one more column once its step is due and ends the reveal after the last one. A wipe
   * whose freeze a later one took over ends without touching the reveal.
   * @param {Display&} display - display the reveal is set on
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void tick(Display& display, uint32_t nowMs);

  /**
   * Ends the wipe where it stands so the new picture shows at once, a reveal a later freeze took
   * over is left alone.
   * @param {Display&} display - display to release
   * @returns {void}
   */
  void stop(Display& display);

private:
  /** Wait before each column, the pace of the layout slide's wipe */
  static constexpr uint32_t kColumnMs = 35;

  /**
   * Returns the reveal mask that shows the first columns from one canvas edge.
   * @param {uint8_t} columns - columns of the new picture on screen, 0 to the canvas width
   * @param {bool} fromLeft - true counts the columns from the left edge
   * @returns {Frame} - mask for Display::setReveal()
   */
  static Frame mask(uint8_t columns, bool fromLeft);

  /**
   * Returns whether the reveal in effect is still the one start() froze.
   * @param {const Display&} display - display to check
   * @returns {bool} - false once a later freeze took it over or the wipe ended
   */
  bool ownsReveal(const Display& display) const;

  uint32_t dueMs_ = 0;
  uint8_t columns_ = 0;
  /** freezeCount() right after the wipe's own freeze */
  uint8_t freezeId_ = 0;
  bool fromLeft_ = false;
  bool running_ = false;
};
