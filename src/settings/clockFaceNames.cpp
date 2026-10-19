#include "settings/clockFaceNames.h"

#include "nameTable.h"

#include <cstddef>

namespace {

// index matches the ClockFace enumerator value
constexpr const char* kNames[] = {"staircase", "tall"};
constexpr size_t kCount = nameCount(kNames);

}

const char* clockFaceName(ClockFace face) {
  return nameAt(kNames, kCount, static_cast<size_t>(face), "unknown");
}

bool parseClockFace(const char* name, ClockFace& out) {
  const int index = parseName(kNames, kCount, name);

  if (index < 0) {
    return false;
  }

  out = static_cast<ClockFace>(index);
  return true;
}
