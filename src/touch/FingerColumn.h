#pragma once

#include "touch/OneEuroFilter.h"

/**
 * Turns the normalised signals of both coins into the filtered finger column: the position
 * estimate of fingerPosition.h, a slew limit of kMaxStep columns per sample that cuts single
 * sample spikes, then a One Euro filter that is heavy at rest and light while moving.
 */
class FingerColumn {
public:
  /**
   * Configures the filter for the sample interval, slew and filter start unprimed.
   * @param {float} sampleS - fixed interval between two updates in seconds
   */
  explicit FingerColumn(float sampleS);

  /**
   * Unprimes the slew limit and the filter, the column itself is kept.
   * @returns {void}
   */
  void reset();

  /**
   * Estimates, limits and filters the column of one sample. Without a signal on either coin the
   * last filtered column is fed through the filter again.
   * @param {float} left - normalised signal of the left coin
   * @param {float} right - normalised signal of the right coin
   * @returns {float} - filtered column 0 to 15
   */
  float update(float left, float right);

  /**
   * Returns the filtered column of the last update.
   * @returns {float} - column 0 to 15
   */
  float value() const;

private:
  /** most columns the estimate may move per sample */
  static constexpr float kMaxStep = 1.5f;
  static constexpr float kMinCutoffHz = 2.5f;
  /** cutoff added per column per second of filtered speed */
  static constexpr float kBeta = 0.12f;
  static constexpr float kSpeedCutoffHz = 1.0f;

  /**
   * Moves the limited column at most kMaxStep towards an estimate, the first one seeds it.
   * @param {float} x - unlimited column estimate
   * @returns {float} - limited column
   */
  float limit(float x);

  OneEuroFilter filter_;
  float slewX_ = 0.0f;
  float value_ = 0.0f;
  bool slewPrimed_ = false;
};
