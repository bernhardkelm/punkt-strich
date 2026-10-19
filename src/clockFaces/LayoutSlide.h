#pragma once

#include "Display.h"
#include "Frame.h"
#include "settings/Settings.h"

#include <cstdint>

/**
 * Steps the clock between the full canvas face and the split clock in the left half of a split
 * picture in whole pixels. Towards the split the clock walks the path of the split clock's style
 * from splitPath, the Tall face squashes into the Staircase one and the minutes move into the
 * split clock, then the split picture wipes in from the viewport's left edge; the way back runs
 * the same frames in reverse. One position per step counts from the full Tall face to the split
 * picture, the owner starts its picture when tick() asks for it.
 */
class LayoutSlide {
public:
  /** What the owner does after a tick */
  enum class Step : uint8_t { NONE, START_PICTURE };

  /**
   * Starts a slide. Towards the split it draws the full face, towards the full face it freezes the
   * split picture on screen and draws the clock in the left half behind it.
   * @param {Display&} display - canvas the slide draws on, viewport at Display::kViewportX
   * @param {bool} toSplit - true slides from the full face into the split picture
   * @param {uint8_t} faceRows - digit height of the full face, from clockFaceRows()
   * @param {SplitClock} style - split clock style, held for the whole slide
   * @param {uint8_t} hour - 0 to 23, held for the whole slide
   * @param {uint8_t} minute - 0 to 59, held for the whole slide
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void start(Display& display, bool toSplit, uint8_t faceRows, SplitClock style, uint8_t hour,
             uint8_t minute, uint32_t nowMs);

  /**
   * Moves the slide one position on once its step is due, at most one step per call. The owner
   * starts its picture under the frozen clock before the wipe into the split, and on the full
   * face the way back ends on.
   * @param {Display&} display - canvas the slide draws on
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {Step} - START_PICTURE when the owner starts its picture now, NONE otherwise
   */
  Step tick(Display& display, uint32_t nowMs);

  /**
   * Ends the slide where it stands and drops its reveal, one a later freeze took over is left.
   * @param {Display&} display - canvas the slide draws on
   * @returns {void}
   */
  void stop(Display& display);

  /**
   * Returns whether a slide runs.
   * @returns {bool} - true from start() until the last step or stop()
   */
  bool running() const;

  /**
   * Returns whether the owner's picture already runs beneath the slide, which is the case while
   * the split picture wipes in.
   * @returns {bool} - true from START_PICTURE towards the split until the slide ends
   */
  bool pictureRuns() const;

private:
  /** Wait before each viewport column the split picture wipes in or out */
  static constexpr uint32_t kWipeColumnMs = 35;

  /**
   * Returns the position the full face of a digit height sits at.
   * @param {uint8_t} faceRows - digit height of the full face
   * @returns {uint8_t} - 0 for the Tall face, kSquashSteps for the Staircase one
   */
  static uint8_t fullPosition(uint8_t faceRows);

  /**
   * Returns the position of the split picture, the wipe's last column.
   * @returns {uint8_t} - wipePosition_ plus the viewport width
   */
  uint8_t splitPosition() const;

  /**
   * Returns the reveal mask that shows the first viewport columns of the split picture, either
   * as the animation frame (wipe in) or by leaving the frozen frame (wipe out).
   * @param {uint8_t} columns - viewport columns of the split picture on screen, 0 to kWidth
   * @param {bool} splitLive - true while the split picture is the animation frame
   * @returns {Frame} - mask for Display::setReveal()
   */
  static Frame wipeMask(uint8_t columns, bool splitLive);

  /**
   * Draws and pushes the clock frame of a position up to wipePosition_.
   * @param {Display&} display - canvas to draw on
   * @param {uint8_t} position - 0 (Tall face) to wipePosition_ (split clock in the left half)
   * @returns {void}
   */
  void drawClock(Display& display, uint8_t position) const;

  /**
   * Applies a position reached on the way into the split picture.
   * @param {Display&} display - canvas to draw on
   * @returns {Step} - START_PICTURE once the clock reached the left half and is frozen
   */
  Step stepTowardSplit(Display& display);

  /**
   * Applies a position reached on the way back to the full face.
   * @param {Display&} display - canvas to draw on
   * @returns {Step} - START_PICTURE once the full face is drawn
   */
  Step stepTowardFull(Display& display);

  /**
   * Returns the wait before the next step, set by the stage that step moves through.
   * @returns {uint32_t} - splitPathStepMs() of the clock stages, kWipeColumnMs in the wipe
   */
  uint32_t nextStepMs() const;

  /**
   * Freezes the picture on screen and remembers the freeze as the slide's own.
   * @param {Display&} display - display to freeze
   * @returns {void}
   */
  void holdReveal(Display& display);

  /**
   * Returns whether the reveal in effect is still the one holdReveal() started.
   * @param {const Display&} display - display to check
   * @returns {bool} - false once a later freeze took it over or nothing is held
   */
  bool ownsReveal(const Display& display) const;

  /**
   * Ends the slide's own reveal, a reveal a later freeze took over is left alone.
   * @param {Display&} display - display to release
   * @returns {void}
   */
  void releaseReveal(Display& display);

  uint32_t dueMs_ = 0;
  uint8_t position_ = 0;
  uint8_t target_ = 0;
  /** Position of the split clock beside a dark viewport, where the wipe starts */
  uint8_t wipePosition_ = 0;
  SplitClock style_ = Settings::kDefaultSplitClock;
  uint8_t hour_ = 0;
  uint8_t minute_ = 0;
  /** freezeCount() right after the slide's own freeze */
  uint8_t freezeId_ = 0;
  bool holdsReveal_ = false;
  bool running_ = false;
};
