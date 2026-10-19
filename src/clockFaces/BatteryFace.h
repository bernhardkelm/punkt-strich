#pragma once

#include "Animation.h"
#include "Frame.h"

#include <cstdint>

/**
 * Full canvas battery view of a board with VBUS sense, a battery outline over the 16x8 canvas. On
 * battery the inside fills from the left with the estimated charge and the percentage sits centred
 * on it, inverted where the fill runs under it, on USB a lightning bolt replaces both. Static, it
 * redraws only when USB presence or the percentage changed.
 */
class BatteryFace : public Animation {
public:
  const char* name() const override;
  void begin(Display& display) override;
  void tick(Display& display, uint32_t nowMs) override;

private:
  /** What the face shows. */
  struct Look {
    /** USB power is present, the bolt replaces fill and digits */
    bool usb = false;
    /** charge in percent, -1 while no estimate exists */
    int8_t percent = -1;
  };

  /**
   * Returns what the face shows now, USB presence and the charge estimate of VbusSense.
   * @returns {Look} - what to draw
   */
  static Look currentLook();

  /**
   * Returns the picture of a look: the outline with the bolt on USB, with the fill and the digits
   * XOR-ed into its inside on battery, and bare while no estimate exists yet.
   * @param {const Look&} look - what to draw
   * @returns {Frame} - canvas picture
   */
  static Frame frameFor(const Look& look);

  /**
   * Replaces the canvas with the picture of a look and pushes the frame.
   * @param {Display&} display - canvas to draw on
   * @param {const Look&} look - what to draw
   * @returns {void}
   */
  static void draw(Display& display, const Look& look);

  Look shown_;
};
