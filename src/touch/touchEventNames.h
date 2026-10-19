#pragma once

#include "touch/TouchEvent.h"

/**
 * Returns the lowercase name of a touch event.
 * @param {TouchEvent} event - event to name
 * @returns {const char*} - "none", "tap", "hold", "double-tap" or "tap-confirmed"
 */
const char* touchEventName(TouchEvent event);
