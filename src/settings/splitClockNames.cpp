#include "settings/splitClockNames.h"

#include "nameTable.h"

#include <cstddef>

namespace {

// index matches the SplitClock enumerator value
constexpr const char* kNames[] = {"digits", "squares", "stagger", "bar"};
constexpr size_t kCount = nameCount(kNames);

}

const char* splitClockName(SplitClock style) {
  return nameAt(kNames, kCount, static_cast<size_t>(style), "unknown");
}

bool parseSplitClock(const char* name, SplitClock& out) {
  const int index = parseName(kNames, kCount, name);

  if (index < 0) {
    return false;
  }

  out = static_cast<SplitClock>(index);
  return true;
}
