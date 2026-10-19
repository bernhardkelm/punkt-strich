#pragma once

#include "touch/Coin.h"

#include <cstdint>

class Display;

/** Part of the canvas a ripple reveals the new picture on. */
enum class RippleScope : uint8_t { FULL, HALF };

/**
 * Draws the touch proximity domes over the running picture and bursts one into a ripple.
 * Each coin owns a dome, a disc around the point between its half's columns 3 and 4 just above
 * row 0, one radius per proximity level, with a dark keyline around it so it stays readable over
 * any animation. Both domes may show at once. A
 * gesture that switches the picture freezes the old frame, and the same disc grows on over the
 * far corners of the clip region, revealing the new picture behind a lit front. A swipe grows
 * that disc out of its coin's dome as the finger moves and runs on as the ripple or shrinks back
 * when released. While a system picture owns the matrix or the feature is off, nothing is drawn.
 */
class TouchOverlay {
public:
  /**
   * Binds the overlay to the display carrying the overlay, reveal and brightness floor layers.
   * @param {Display&} display - display every draw goes to
   */
  explicit TouchOverlay(Display& display);

  /**
   * Turns dome and ripple on or off, switching off drops the layers and the brightness floor.
   * @param {bool} enabled - false while the touchHover setting is 0
   * @returns {void}
   */
  void setEnabled(bool enabled);

  /**
   * Freezes the picture on screen so a gesture can reveal the next one and floors the brightness
   * at its level, called right before the gesture is applied.
   * @returns {void}
   */
  void gestureBegins();

  /**
   * Starts the ripple when the gesture changed the picture and drops the frozen frame otherwise,
   * called right after the gesture was applied.
   * @param {bool} pictureChanged - true when another animation is running now
   * @param {Coin} coin - coin whose dome the ripple grows from
   * @param {RippleScope} scope - FULL reveals the whole canvas, HALF only the coin's half
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void gestureEnds(bool pictureChanged, Coin coin, RippleScope scope, uint32_t nowMs);

  /**
   * Freezes the picture on screen like gestureBegins() and starts the swipe disc over the whole
   * canvas at the dome radius it grows out of, called right before the swipe switches the view.
   * @param {Coin} coin - coin the swipe started on, the disc grows from its dome
   * @param {float} floorRadius - dome radius in doubled units the disc starts at and retracts to
   * @returns {bool} - true when the disc runs, false while the overlay is off or blocked
   */
  bool swipeBegins(Coin coin, float floorRadius);

  /**
   * Moves the front of the running swipe disc, drawn on the next tick, never below its floor.
   * @param {float} radius - disc radius in doubled units
   * @returns {void}
   */
  void swipeTo(float radius);

  /**
   * Releases the swipe disc at the ripple speed and easing: a complete swipe runs on as the
   * ripple to the farthest corner, a short one shrinks back to its floor radius and holds the
   * frozen picture there until endSwipe().
   * @param {bool} complete - true when the swipe keeps the new picture
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void swipeEnds(bool complete, uint32_t nowMs);

  /**
   * Returns whether a swipe disc still follows the finger or shrinks back, a short swipe's view
   * is restored once this turned false.
   * @returns {bool} - true until a short swipe's disc is back at its floor or was dropped
   */
  bool retracting() const;

  /**
   * Ends the reveal of a retracted swipe, called once the view it switched to was restored.
   * @returns {void}
   */
  void endSwipe();

  /**
   * Drives the ripple or the swipe disc, or draws the domes of the current proximity levels, call
   * from loop() before the state machine ticks.
   * @param {uint8_t} leftLevel - left coin proximity, 0 for none up to 3 for contact
   * @param {uint8_t} rightLevel - right coin proximity, always 0 without a right half
   * @param {bool} blocked - true while a system picture owns the matrix
   * @param {uint8_t} ceiling - current brightness ceiling, 0 to 15
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void tick(uint8_t leftLevel, uint8_t rightLevel, bool blocked, uint8_t ceiling, uint32_t nowMs);

private:
  /** What the swipe disc does, NONE also while a plain ripple runs. */
  enum class SwipePhase : uint8_t { NONE, FOLLOW, RETRACT, HELD };

  static constexpr uint8_t kLevels = 4;
  /** squared dome radius per proximity level in doubled units, level 0 draws nothing */
  static constexpr uint16_t kLevelR2[kLevels] = {0, 2, 10, 34};
  /** brightness the dome is floored at, so it shows on the off view running at level 1 */
  static constexpr uint8_t kDomeFloor = 4;
  /** ripple duration on the 8x8 canvas, the reveal speed it sets holds on every canvas width */
  static constexpr uint32_t kReferenceRippleMs = 240;
  /** canvas width the reference duration was tuned on */
  static constexpr uint8_t kReferenceWidth = 8;
  /** width of the lit front in doubled units, 2 is one pixel */
  static constexpr float kFrontWidth = 2.0f;
  /** ripple start radius in doubled units, the edge of the level 3 dome at d2 = 34 */
  static constexpr float kStartRadius = 5.830952f;

  /** columns per coin half */
  static constexpr uint8_t kHalfWidth = 8;

  /**
   * Draws the domes of the proximity levels, or clears them while both levels are 0, the matrix is
   * blocked or the overlay is off.
   * @param {uint8_t} leftLevel - left coin proximity, 0 for none up to 3 for contact
   * @param {uint8_t} rightLevel - right coin proximity, 0 for none up to 3 for contact
   * @returns {void}
   */
  void drawDome(uint8_t leftLevel, uint8_t rightLevel);

  /**
   * Drops the dome and its brightness floor.
   * @returns {void}
   */
  void clearDome();

  /**
   * Starts a timed disc run from one radius to another at the reference reveal speed.
   * @param {float} from - start radius in doubled units
   * @param {float} to - end radius in doubled units
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void startRipple(float from, float to, uint32_t nowMs);

  /**
   * Draws the running ripple or the swipe disc.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {bool} - true while a reveal runs, false once it ended
   */
  bool drawReveal(uint32_t nowMs);

  /**
   * Draws the disc for the eased, elapsed share of rippleMs_, a finished retract holds its floor.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {bool} - true while the ripple runs or the retract holds, false once it ended
   */
  bool drawRipple(uint32_t nowMs);

  /**
   * Draws the revealed disc of a radius around the origin inside the clip region and its front.
   * @param {float} radius - disc radius in doubled units
   * @returns {void}
   */
  void drawDisc(float radius);

  /**
   * Ends the ripple and the swipe disc, the caller sets the brightness floor back to the dome rule.
   * @returns {void}
   */
  void endRipple();

  /**
   * Returns the brightness floor of the dome, held under the ceiling so a dimmed picture stays
   * dimmed.
   * @returns {uint8_t} - floor level, 0 to 15
   */
  uint8_t domeFloor() const;

  Display& display_;
  /** radius the timed disc run starts at, in doubled units */
  float rippleFrom_ = 0.0f;
  /** radius the timed disc run ends at, the farthest clip corner plus the front width */
  float rippleTo_ = 0.0f;
  /** radius the swipe disc follows the finger at, in doubled units */
  float swipeRadius_ = 0.0f;
  /** dome radius the swipe disc grew out of, in doubled units */
  float swipeFloor_ = 0.0f;
  /** columns the ripple origin is shifted right */
  uint8_t originX_ = 0;
  uint8_t clipFirst_ = 0;
  uint8_t clipLast_ = 0;
  /** ripple duration in ms, scaled so the reveal speed matches the reference canvas */
  uint32_t rippleMs_ = 0;
  uint32_t rippleStartMs_ = 0;
  /** brightness floor held from the freeze through the ripple, the frozen frame's level at least */
  uint8_t rippleFloor_ = 0;
  /** brightness ceiling of the last tick */
  uint8_t ceiling_ = 0;
  /** blocked flag of the last tick, a gesture under a system picture must not freeze */
  bool blocked_ = false;
  bool enabled_ = true;
  /** the picture was frozen and the gesture may still turn into a ripple */
  bool armed_ = false;
  bool rippling_ = false;
  SwipePhase swipePhase_ = SwipePhase::NONE;
};
