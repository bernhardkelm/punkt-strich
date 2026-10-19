#pragma once

#include "Animation.h"
#include "SplitPicture.h"
#include "clockFaces/LayoutSlide.h"
#include "clockFaces/WideClock.h"
#include "settings/Settings.h"

#include <cstdint>
#include <memory>

/**
 * Round trip of the strich layout slide: the full clock face, the slide into the split picture at
 * kToSplitMs, the split picture until kToFaceMs and the slide back. LayoutSlide steps at its own
 * pace and starts each picture the way the state machine does.
 */
class SlideRoundTrip : public Animation {
public:
  /**
   * Creates the round trip.
   * @param {const Settings&} settings - source of face and split clock style, must outlive it
   * @param {std::unique_ptr<Animation>} picture - Claude picture of the split picture
   */
  SlideRoundTrip(const Settings& settings, std::unique_ptr<Animation> picture);

  const char* name() const override;
  void begin(Display& display) override;
  void tick(Display& display, uint32_t nowMs) override;

private:
  /** millis() the slide into the split picture starts at */
  static constexpr uint32_t kToSplitMs = 1000;
  /** millis() the slide back to the full face starts at */
  static constexpr uint32_t kToFaceMs = 5200;

  const Settings& settings_;
  WideClock face_;
  SplitPicture split_;
  LayoutSlide slide_;
  /** picture the round trip runs or the running slide leads to */
  Animation* shown_ = nullptr;
  bool toSplitStarted_ = false;
  bool toFaceStarted_ = false;

  /**
   * Starts a slide at the render time, the picture it leads to becomes the shown one.
   * @param {Display&} display - canvas the slide draws on
   * @param {bool} toSplit - true slides into the split picture
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void startSlide(Display& display, bool toSplit, uint32_t nowMs);

  /**
   * Moves a running slide on and begins the shown picture when the slide asks for it.
   * @param {Display&} display - canvas the slide draws on
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {bool} - true while the shown picture must not tick
   */
  bool advanceSlide(Display& display, uint32_t nowMs);
};
