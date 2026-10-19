#pragma once

#include "touch/Coin.h"

/**
 * Radius of the disc a swipe grows out of its start coin's dome, in doubled units around the
 * coin centre. It follows the finger only in the swipe direction, so a finger behind the coin
 * centre leaves it at the dome radius, and closes kFollow of the gap per sample.
 */
class SwipeDisc {
public:
  /**
   * Starts the disc at the dome radius it grows out of.
   * @param {Coin} coin - start coin of the swipe
   * @param {float} floor - dome radius of that coin, the disc never shrinks below it
   * @returns {void}
   */
  void begin(Coin coin, float floor);

  /**
   * Moves the radius one sample towards the one that puts the front at the finger column.
   * @param {float} x - filtered finger column
   * @returns {void}
   */
  void follow(float x);

  /**
   * Returns the start coin of the last swipe.
   * @returns {Coin} - coin the disc grows from
   */
  Coin coin() const;

  /**
   * Returns the current radius.
   * @returns {float} - radius in doubled units
   */
  float radius() const;

  /**
   * Returns the dome radius the disc grew out of.
   * @returns {float} - radius in doubled units
   */
  float floor() const;

private:
  /** share of the gap to the finger radius the disc closes per sample */
  static constexpr float kFollow = 0.45f;

  /**
   * Returns the radius that puts the front at a column, counted only ahead of the coin centre.
   * @param {float} x - finger column
   * @returns {float} - 2 * columns ahead + 1, never below the floor
   */
  float radiusAt(float x) const;

  Coin coin_ = Coin::LEFT;
  float radius_ = 0.0f;
  float floor_ = 0.0f;
};
