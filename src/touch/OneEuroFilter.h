#pragma once

/**
 * One Euro low pass for a value sampled at a fixed interval: the cutoff rises with the filtered
 * speed, so the value is smoothed heavily at rest and follows with little lag while moving.
 */
class OneEuroFilter {
public:
  /**
   * Configures the filter, it starts unprimed.
   * @param {float} minCutoffHz - cutoff at rest in Hz
   * @param {float} beta - cutoff added per unit of speed per second
   * @param {float} speedCutoffHz - cutoff of the speed estimate in Hz
   * @param {float} sampleS - fixed interval between two samples in seconds
   */
  OneEuroFilter(float minCutoffHz, float beta, float speedCutoffHz, float sampleS);

  /**
   * Unprimes the filter, the next sample passes through unfiltered and seeds it.
   * @returns {void}
   */
  void reset();

  /**
   * Feeds one sample through the filter.
   * @param {float} raw - unfiltered value
   * @returns {float} - filtered value
   */
  float filter(float raw);

private:
  /**
   * Returns the weight of a new sample in a first order low pass at the sample interval.
   * @param {float} cutoffHz - cutoff frequency
   * @returns {float} - weight 0 to 1
   */
  float alphaFor(float cutoffHz) const;

  float minCutoffHz_;
  float beta_;
  float speedCutoffHz_;
  float sampleS_;
  float smooth_ = 0.0f;
  float speed_ = 0.0f;
  float lastRaw_ = 0.0f;
  bool primed_ = false;
};
