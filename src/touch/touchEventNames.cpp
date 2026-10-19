#include "touch/touchEventNames.h"

#include "nameTable.h"

#include <cstddef>

namespace {

// index matches the TouchEvent enumerator value
constexpr const char* kNames[] = {"none", "tap", "hold", "double-tap", "tap-confirmed"};
constexpr size_t kCount = nameCount(kNames);

}

const char* touchEventName(TouchEvent event) {
  return nameAt(kNames, kCount, static_cast<size_t>(event), "none");
}
