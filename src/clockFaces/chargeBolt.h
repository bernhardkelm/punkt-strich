#pragma once

#include "Frame.h"

/**
 * Returns the lightning bolt of the battery face on USB, 5 columns by 6 rows centred inside the
 * battery outline, a zigzag from a tip at the top right to a tip at the bottom left.
 * @returns {Frame} - frame with the bolt pixels set
 */
Frame chargeBolt();
