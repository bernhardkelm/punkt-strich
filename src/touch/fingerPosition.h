#pragma once

#include "touch/Coin.h"

/**
 * Returns the canvas column under a coin's centre.
 * @param {Coin} coin - coin to locate
 * @returns {float} - 3.5 for the left coin above columns 0 to 7, 11.5 for the right one
 */
float coinCentre(Coin coin);

/**
 * Normalises a coin's smoothed delta for the position estimate: a noise floor of 10 percent of
 * the full touch delta is taken off, the rest is divided by the larger of the gesture peak and
 * the full touch delta.
 * @param {float} delta - smoothed delta above the baseline in raw counts
 * @param {float} gestureMax - highest delta of the running gesture in raw counts
 * @param {float} touchDelta - full touch delta, the baseline times the touch share
 * @returns {float} - signal from 0, 0 at or below the noise floor
 */
float normalisedSignal(float delta, float gestureMax, float touchDelta);

/**
 * Estimates the finger column from both normalised signals. Each coin places the finger between
 * its centre and its outer edge, where it shows one third of its centre signal, and the two
 * places are weighted by their signals.
 * @param {float} left - normalised signal of the left coin
 * @param {float} right - normalised signal of the right coin
 * @returns {float} - column 0 to 15, only defined while left + right is above 0
 */
float columnFrom(float left, float right);
