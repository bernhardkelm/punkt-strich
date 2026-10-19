#include "settings/workingAnimationNames.h"

#include "nameTable.h"

#include <cstddef>

namespace {

// index matches the WorkingAnimation enumerator value
constexpr const char* kNames[] = {"tetris", "life",  "knight",  "orbit", "code",     "rain",
                                  "scan",   "pipes", "pointer", "pong",  "terminal"};
constexpr size_t kCount = nameCount(kNames);

}

const char* workingAnimationName(WorkingAnimation animation) {
  return nameAt(kNames, kCount, static_cast<size_t>(animation), "unknown");
}

bool parseWorkingAnimation(const char* name, WorkingAnimation& out) {
  const int index = parseName(kNames, kCount, name);

  if (index < 0) {
    return false;
  }

  out = static_cast<WorkingAnimation>(index);
  return true;
}
