#pragma once

#include <cstddef>

/** One place the board can sit in, the timezone rule and the coordinates derive from it. */
struct City {
  /** IANA name such as "Europe/Berlin", the value the city setting stores. */
  const char* name;
  /** Display name for the settings page. */
  const char* label;
  /** Group the settings page lists the city under. */
  const char* region;
  /** POSIX TZ rule the clock runs on. */
  const char* posix;
  /** City centre latitude in degrees, north positive. */
  float latitude;
  /** City centre longitude in degrees, east positive. */
  float longitude;
};

/**
 * Looks a city up by its IANA name.
 * @param {const char*} name - candidate name, may be null
 * @returns {const City*} - table entry, nullptr for a null or unknown name
 */
const City* findCity(const char* name);

/**
 * Returns how many cities the table holds.
 * @returns {size_t} - entries in the table
 */
size_t cityCount();

/**
 * Returns one table entry by position, the order the settings page lists them in.
 * @param {size_t} index - position, values past the end are clamped to the last entry
 * @returns {const City&} - table entry
 */
const City& cityAt(size_t index);

/**
 * Returns the city a board without a stored setting sits in.
 * @returns {const City&} - Europe/Berlin
 */
const City& defaultCity();
