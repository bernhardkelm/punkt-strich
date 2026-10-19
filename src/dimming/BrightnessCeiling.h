#pragma once

#include <cstdint>

class AutoDim;
class RestDim;
class Settings;
class StateMachine;

/**
 * Hands the state machine the brightness ceiling restCeiling() computes from the sun level AutoDim
 * evaluates and the resting flag RestDim evaluates, and remembers it for the touch overlay.
 */
class BrightnessCeiling {
public:
  /**
   * Binds the ceiling to its inputs and the state machine it drives, nothing is evaluated yet.
   * @param {Settings&} settings - night brightness, auto dim and dim timeout settings
   * @param {StateMachine&} state - state machine the ceiling and the dimmed flag are handed to
   * @param {AutoDim&} autoDim - sun level source
   * @param {RestDim&} restDim - resting flag source
   */
  BrightnessCeiling(Settings& settings, StateMachine& state, AutoDim& autoDim, RestDim& restDim);

  /**
   * Re-evaluates the sun level and the resting flag right away and applies the ceiling with the
   * default fade, the path a boot and a settings change take.
   * @returns {void}
   */
  void refresh();

  /**
   * Evaluates the sun level and the resting flag on their own schedules and applies the ceiling
   * when either changed, gliding into a rest with RestDim::kRampMs.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void tick(uint32_t nowMs);

  /**
   * Returns the ceiling last handed to the state machine, the overlay floors against it.
   * @returns {uint8_t} - brightness ceiling, 0 to 15
   */
  uint8_t level() const;

private:
  /**
   * Applies the brightness ceiling from the sun level AutoDim last evaluated and the resting flag
   * RestDim last evaluated, and mirrors that resting flag onto the state machine's dimmed
   * indicator.
   * @param {uint32_t} fadeMs - ramp duration handed to state.setBrightness()
   * @returns {void}
   */
  void apply(uint32_t fadeMs);

  Settings& settings_;
  StateMachine& state_;
  AutoDim& autoDim_;
  RestDim& restDim_;
  uint8_t applied_;
};
