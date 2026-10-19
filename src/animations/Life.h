#pragma once

#include "Animation.h"

/** Conway's Game of Life on a wrapping 8x8 grid, reseeds when it dies, stalls or runs long enough. */
class Life : public Animation {
public:
  const char* name() const override;
  void begin(Display& display) override;
  void tick(Display& display, uint32_t nowMs) override;

private:
  using Grid = uint8_t[Display::kHeight];

  static constexpr uint32_t kStepMs = 400;
  static constexpr uint8_t kSeedDensityPercent = 15;
  static constexpr uint16_t kMaxGenerations = 60;

  Grid rows_ = {};
  /** the two generations before rows_, a repeat of either is a still life or a period 2 oscillator */
  Grid previous_ = {};
  Grid beforePrevious_ = {};
  uint32_t nextStepMs_ = 0;
  uint16_t generation_ = 0;

  /** Fills the grid with random cells and restarts the generation count. */
  void seed();

  /** Advances the grid by one generation, keeping the two generations before it. */
  void step();

  /**
   * Returns whether no cell is alive.
   * @returns {bool} - true when the grid is empty
   */
  bool isDead() const;

  /**
   * Returns whether the grid stopped evolving, a still life or a period 2 oscillator.
   * @returns {bool} - true when the current generation equals one of the two before it
   */
  bool isStuck() const;

  /**
   * Computes the next generation of a grid.
   * @param {const Grid&} from - current cells, one bit per column
   * @param {Grid&} to - receives the next generation
   * @returns {void}
   */
  static void evolve(const Grid& from, Grid& to);

  /**
   * Counts the live neighbours of a cell, wrapping at the edges.
   * @param {const Grid&} grid - cells to inspect
   * @param {uint8_t} x - column
   * @param {uint8_t} y - row
   * @returns {uint8_t} - 0 to 8
   */
  static uint8_t neighbours(const Grid& grid, uint8_t x, uint8_t y);

  /**
   * Draws the current generation.
   * @param {Display&} display - matrix to draw on
   * @returns {void}
   */
  void draw(Display& display) const;
};
