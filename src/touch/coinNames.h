#pragma once

#include "touch/Coin.h"

/**
 * Returns the name a coin is logged with.
 * @param {Coin} coin - coin to name
 * @returns {const char*} - "left" or "right"
 */
const char* coinName(Coin coin);
