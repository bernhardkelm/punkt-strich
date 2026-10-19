#pragma once

#include <cstddef>
#include <cstring>

/**
 * Returns how many names a table holds.
 * @param {const char* const (&)[N]} table - names indexed by an enumerator value
 * @returns {size_t} - entries in the table
 */
template <size_t N>
constexpr size_t nameCount(const char* const (&)[N]) {
  return N;
}

/**
 * Returns the name an enumerator value stands for.
 * @param {const char* const*} table - names indexed by the enumerator value
 * @param {size_t} count - entries in the table
 * @param {size_t} index - enumerator value to name
 * @param {const char*} fallback - name for a value outside the table
 * @returns {const char*} - entry of the table or fallback
 */
inline const char* nameAt(const char* const* table, size_t count, size_t index,
                          const char* fallback) {
  if (index >= count) {
    return fallback;
  }

  return table[index];
}

/**
 * Looks a name up in an index matched table.
 * @param {const char* const*} table - names indexed by the enumerator value
 * @param {size_t} count - entries in the table
 * @param {const char*} name - candidate name, may be null
 * @returns {int} - enumerator value of the entry, -1 when the name is unknown
 */
inline int parseName(const char* const* table, size_t count, const char* name) {
  if (name == nullptr) {
    return -1;
  }

  for (size_t i = 0; i < count; i++) {
    if (strcmp(name, table[i]) == 0) {
      return static_cast<int>(i);
    }
  }

  return -1;
}
