#include "settings/activityNames.h"

#include "nameTable.h"

#include <cstddef>

namespace {

// index matches the Activity enumerator value
constexpr const char* kNames[] = {"thinking", "reading", "editing", "shell", "delegating", "mcp"};
constexpr size_t kCount = nameCount(kNames);

}

const char* activityName(Activity activity) {
  return nameAt(kNames, kCount, static_cast<size_t>(activity), "unknown");
}

bool parseActivity(const char* name, Activity& out) {
  const int index = parseName(kNames, kCount, name);

  if (index < 0) {
    return false;
  }

  out = static_cast<Activity>(index);
  return true;
}
