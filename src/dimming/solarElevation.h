#pragma once

#include <ctime>

/**
 * Returns the sun's elevation above the horizon for a place and a moment, NOAA low precision
 * algorithm, accurate to about half a degree. The timezone plays no part, the moment is UTC.
 * @param {time_t} utc - Unix time of the moment
 * @param {float} latitudeDeg - latitude in degrees, north positive
 * @param {float} longitudeDeg - longitude in degrees, east positive
 * @returns {float} - elevation in degrees, negative below the horizon
 */
float solarElevationDeg(time_t utc, float latitudeDeg, float longitudeDeg);
