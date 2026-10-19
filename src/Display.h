#pragma once

#include "BrightnessRamp.h"
#include "Frame.h"
#include "board/boardConfig.h"

#include <Adafruit_LEDBackpack.h>
#include <cstdint>

/**
 * Thin wrapper around the HT16K33 8x8 backpacks, the only place that touches the Adafruit
 * library. The backpacks sit side by side and form one canvas of kCanvasWidth x kHeight pixels.
 * Holds the animation frame plus an overlay and a reveal layer across the canvas, composed into
 * one picture on every push. Animations draw into a kWidth x kHeight viewport that starts at
 * canvas column kViewportX, setViewportX() moves it.
 */
class Display {
public:
  /** Viewport width, the columns animations draw into */
  static constexpr uint8_t kWidth = 8;
  /** Viewport and canvas height */
  static constexpr uint8_t kHeight = 8;
  /** Canvas width across all backpacks, left to right */
  static constexpr uint8_t kCanvasWidth = Board::kCanvasWidth;
  /** Canvas column the viewport starts at until setViewportX() moves it */
  static constexpr uint8_t kViewportX = Board::kViewportX;
  static constexpr uint8_t kMaxBrightness = 15;
  static constexpr uint32_t kFadeMs = 150;

  /**
   * Starts I2C on the given pins and probes every backpack of the board.
   * @param {uint8_t} sda - I2C data pin
   * @param {uint8_t} scl - I2C clock pin
   * @returns {bool} - true when every backpack acknowledged, see failedAddress() otherwise
   */
  bool begin(uint8_t sda, uint8_t scl);

  /**
   * Returns the backpack address the last begin() gave up on, the first address when I2C did not
   * start.
   * @returns {uint8_t} - 7-bit I2C address, 0 when every backpack acknowledged
   */
  uint8_t failedAddress() const;

  /**
   * Turns every viewport pixel off in the animation frame, other canvas columns keep their state,
   * call show() to apply.
   * @returns {void}
   */
  void clear();

  /**
   * Turns every canvas pixel off in the animation frame, call show() to apply.
   * @returns {void}
   */
  void clearCanvas();

  /**
   * Sets one viewport pixel in the animation frame, out-of-range coordinates are ignored.
   * @param {int8_t} x - viewport column, 0 is left
   * @param {int8_t} y - row, 0 is top
   * @param {bool} on - pixel state
   * @returns {void}
   */
  void setPixel(int8_t x, int8_t y, bool on);

  /**
   * Sets one canvas pixel in the animation frame, out-of-range coordinates are ignored.
   * @param {int8_t} x - canvas column, 0 is the left edge of the leftmost backpack
   * @param {int8_t} y - row, 0 is top
   * @param {bool} on - pixel state
   * @returns {void}
   */
  void setCanvasPixel(int8_t x, int8_t y, bool on);

  /**
   * Moves the viewport setPixel() and clear() address, a column that would push it past the right
   * canvas edge is clipped to the rightmost position.
   * @param {uint8_t} x - canvas column of the viewport's left edge
   * @returns {void}
   */
  void setViewportX(uint8_t x);

  /**
   * Composes the layers and pushes the canvas to the backpacks, each one showing its 8 columns.
   * @returns {void}
   */
  void show();

  /**
   * Forces pixels on and off over the composed picture, pushed by tick() when it changes.
   * @param {const Frame&} on - bits forced on, applied last
   * @param {const Frame&} off - bits forced off
   * @returns {void}
   */
  void setOverlay(const Frame& on, const Frame& off);

  /**
   * Drops the overlay layer, pushed by tick() when the picture changes.
   * @returns {void}
   */
  void clearOverlay();

  /**
   * Copies the animation frame into the frozen layer and starts a closed reveal, so the matrix
   * keeps the picture while the next animation draws into the animation frame.
   * @returns {void}
   */
  void freeze();

  /**
   * Opens the frozen layer, set bits show the animation frame and clear bits the frozen one.
   * @param {const Frame&} mask - reveal mask
   * @returns {void}
   */
  void setReveal(const Frame& mask);

  /**
   * Ends the reveal so the animation frame shows again, a no-op while not revealing.
   * @returns {void}
   */
  void endReveal();

  /**
   * Returns whether a frozen frame is held, from freeze() until endReveal().
   * @returns {bool} - true while the reveal mask decides between frozen and animation frame
   */
  bool revealing() const;

  /**
   * Returns a counter every freeze() advances, so a caller that froze the picture can tell whether
   * a later freeze took its reveal over.
   * @returns {uint8_t} - number of freezes, wrapping at 256
   */
  uint8_t freezeCount() const;

  /**
   * Fades to a brightness level over a ramp, applied by tick().
   * @param {uint8_t} level - 0 (dimmest) to 15 (brightest), clamped
   * @param {uint32_t} durationMs - ramp length, 0 applies immediately
   * @returns {void}
   */
  void setBrightness(uint8_t level, uint32_t durationMs = kFadeMs);

  /**
   * Jumps to a brightness level without a ramp, applied by tick().
   * @param {uint8_t} level - 0 (dimmest) to 15 (brightest), clamped
   * @returns {void}
   */
  void setBrightnessNow(uint8_t level);

  /**
   * Holds the brightness at a minimum level, whatever the animations request, and fades over
   * kFadeMs when the floor changes.
   * @param {uint8_t} level - floor level, clamped to kMaxBrightness, 0 removes the floor
   * @returns {void}
   */
  void setBrightnessFloor(uint8_t level);

  /**
   * Returns the brightness the matrix runs at.
   * @returns {uint8_t} - last level written by tick(), the requested level before the first write
   */
  uint8_t brightness() const;

  /**
   * Drives the brightness ramp and pushes a dirty picture, call from loop() as often as possible.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void tick(uint32_t nowMs);

  /**
   * Blanks every backpack and stops its HT16K33 oscillator for the deep sleep, the next begin()
   * starts them again.
   * @returns {void}
   */
  void standby();

private:
  static constexpr uint32_t kI2cHz = 400000;
  /** HT16K33 system setup command with the oscillator bit cleared */
  static constexpr uint8_t kOscillatorOff = 0x20;
  /** Marks that tick() has not written a brightness level yet */
  static constexpr uint8_t kLevelUnset = 0xFF;
  /** Columns driven by one HT16K33 8x8 backpack */
  static constexpr uint8_t kMatrixWidth = 8;
  /** Canvas row bits covered by a viewport at column 0 */
  static constexpr uint16_t kViewportBits = static_cast<uint16_t>((1U << kWidth) - 1U);
  /** Rightmost canvas column the viewport may start at */
  static constexpr uint8_t kMaxViewportX = static_cast<uint8_t>(kCanvasWidth - kWidth);

  static_assert(kCanvasWidth == Board::kMatrixCount * kMatrixWidth,
                "the canvas spans 8 columns per backpack");
  static_assert(kCanvasWidth <= 16, "Frame rows hold at most 16 columns");
  static_assert(kViewportX + kWidth <= kCanvasWidth, "the viewport must fit inside the canvas");

  /**
   * Composes the animation frame, the reveal layer and the overlay into the picture to push.
   * @returns {Frame} - composed picture
   */
  Frame compose() const;

  /**
   * Returns the level to ramp to, the requested level lifted by the floor.
   * @returns {uint8_t} - effective brightness level
   */
  uint8_t effectiveBrightness() const;

  /**
   * Resets the animation frame, the frozen frame, the overlay and the reveal layer.
   * @returns {void}
   */
  void resetLayers();

  /**
   * Drops the overlay and the reveal layer, leaving the animation frame untouched.
   * @returns {void}
   */
  void dropOverlayAndReveal();

  Adafruit_8x8matrix matrices_[Board::kMatrixCount];
  BrightnessRamp brightness_;
  Frame base_;
  Frame frozen_;
  Frame reveal_;
  Frame overlayOn_;
  Frame overlayOff_;
  bool revealing_ = false;
  uint8_t freezeCount_ = 0;
  bool dirty_ = false;
  bool wireStarted_ = false;
  uint8_t failedAddress_ = 0;
  uint8_t requested_ = kMaxBrightness;
  uint8_t floor_ = 0;
  uint8_t writtenLevel_ = kLevelUnset;
  uint8_t viewportX_ = kViewportX;
};
