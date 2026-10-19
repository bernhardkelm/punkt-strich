#include "state/stateNames.h"

#include "nameTable.h"

#include <cstddef>

namespace {

// index matches the PunktState enumerator value
constexpr const char* kNames[] = {"off",        "loading", "idle",   "working",
                                  "attention",  "done",    "error",  "waiting",
                                  "compacting", "limit",   "unread", "glitch"};
constexpr size_t kCount = nameCount(kNames);
// index matches the View enumerator value
constexpr const char* kViewNames[] = {"clock", "normal", "usage", "off", "battery"};
constexpr size_t kViewCount = nameCount(kViewNames);
// index matches the OffReason enumerator value
constexpr const char* kOffReasonNames[] = {"none", "manual", "timeout", "schedule"};
constexpr size_t kOffReasonCount = nameCount(kOffReasonNames);

}

const char* stateName(PunktState state) {
  return nameAt(kNames, kCount, static_cast<size_t>(state), "unknown");
}

bool parseState(const char* name, PunktState& out) {
  const int index = parseName(kNames, kCount, name);

  if (index < 0) {
    return false;
  }

  out = static_cast<PunktState>(index);
  return true;
}

const char* viewName(View view) {
  return nameAt(kViewNames, kViewCount, static_cast<size_t>(view), "unknown");
}

bool parseView(const char* name, View& out) {
  const int index = parseName(kViewNames, kViewCount, name);

  if (index < 0) {
    return false;
  }

  out = static_cast<View>(index);
  return true;
}

const char* offReasonName(OffReason reason) {
  return nameAt(kOffReasonNames, kOffReasonCount, static_cast<size_t>(reason), "unknown");
}
