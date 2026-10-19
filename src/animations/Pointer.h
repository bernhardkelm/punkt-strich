#pragma once

#include "Animation.h"

/**
 * A mouse pointer at work: a three pixel L pointing to the upper left glides to a random spot and
 * rests there. On some arrivals it clicks, which drops the two arm pixels while a ring ripples out
 * of the tip, on others it drags a rectangle open towards a far corner and holds it for a moment
 * before everything clears and the pointer sets off again.
 */
class Pointer : public Animation {
public:
  const char* name() const override;
  void begin(Display& display) override;
  void tick(Display& display, uint32_t nowMs) override;

private:
  static constexpr uint32_t kStepMs = 160;
  static constexpr uint32_t kHoldMs = 500;
  static constexpr uint32_t kIdleMaxMs = 800;
  static constexpr uint8_t kClickPercent = 30;
  static constexpr uint8_t kDragPercent = 20;
  static constexpr uint32_t kClickStepMs = 120;
  /** ring radii the click ripple runs through, radius 1 on step 0 up to radius 5 on step 4 */
  static constexpr uint8_t kRippleRings = 5;
  /** click steps that keep the arms dark, the L reforms from step 3 on */
  static constexpr uint8_t kArmsOffSteps = 3;
  static constexpr uint32_t kDragStepMs = 120;
  static constexpr uint32_t kDragHoldMs = 600;

  /** What the pointer is doing, MOVE and DRAG step it towards the target. */
  enum class Phase : uint8_t { MOVE, CLICK, DRAG, DRAG_HOLD, REST };

  int8_t x_ = 0;
  int8_t y_ = 0;
  int8_t targetX_ = 0;
  int8_t targetY_ = 0;
  int8_t originX_ = 0;
  int8_t originY_ = 0;
  uint8_t clickStep_ = 0;
  uint32_t nextStepMs_ = 0;
  Phase phase_ = Phase::MOVE;

  /**
   * Moves the L one pixel towards its target, diagonally while both axes are off.
   * @returns {void}
   */
  void moveStep();

  /**
   * Rolls once on arrival for a click, a drag or a plain rest, and starts that phase.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {bool} - true when the first frame of the new phase differs from the current one
   */
  bool startAction(uint32_t nowMs);

  /**
   * Rests on the spot for the hold time plus a random idle, so not every stop is equally long.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void startRest(uint32_t nowMs);

  /**
   * Runs the phase whose time is up and starts the next step.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {bool} - true when the frame changed and has to be pushed
   */
  bool advance(uint32_t nowMs);

  /**
   * Draws the drag rectangle and the click ripple the current phase calls for, cursor on top.
   * @param {Display&} display - matrix to draw on
   * @returns {void}
   */
  void draw(Display& display) const;

  /**
   * Draws the tip, plus the two arm pixels unless a click just dropped them.
   * @param {Display&} display - matrix to draw on
   * @returns {void}
   */
  void drawCursor(Display& display) const;
};
