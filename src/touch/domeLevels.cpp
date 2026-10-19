#include "touch/domeLevels.h"

#include <cmath>

namespace {

constexpr uint8_t kLevels = 3;
/** strength a coin must reach to enter the levels 1 to 3 */
constexpr float kLevelEntry[kLevels] = {0.13f, 0.30f, 0.55f};
/** a level is left below this share of its entry strength */
constexpr float kLevelExit = 0.7f;
/** squared dome radius per level 0 to 3 in doubled units */
constexpr uint16_t kLevelR2[kLevels + 1] = {0, 2, 10, 34};

}

uint8_t nextDomeLevel(uint8_t level, float strength) {
  while (level < kLevels && strength >= kLevelEntry[level]) {
    level++;
  }

  while (level > 0 && strength < kLevelEntry[level - 1] * kLevelExit) {
    level--;
  }

  return level;
}

float domeRadius(uint8_t level) {
  const uint8_t clamped = level < kLevels ? level : kLevels;
  return std::sqrt(static_cast<float>(kLevelR2[clamped]));
}
