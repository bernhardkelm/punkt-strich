#include "touch/fingerPosition.h"

#include <algorithm>

namespace {

/** coin centres, the left coin sits above the columns 0 to 7, the right one above 8 to 15 */
constexpr float kCentres[2] = {3.5f, 11.5f};
constexpr float kLastColumn = 15.0f;
/** share of its centre signal a coin shows at its outer edge */
constexpr float kEdgeShare = 1.0f / 3.0f;
/** share of the full touch delta below which a coin adds nothing to the position */
constexpr float kNoiseShare = 0.10f;

}

float coinCentre(Coin coin) {
  return kCentres[static_cast<uint8_t>(coin)];
}

float normalisedSignal(float delta, float gestureMax, float touchDelta) {
  const float peak = gestureMax > touchDelta ? gestureMax : touchDelta;
  const float noise = touchDelta * kNoiseShare;
  return (delta > noise ? delta - noise : 0.0f) / peak;
}

float columnFrom(float left, float right) {
  const float outerL = std::clamp((left - kEdgeShare) / (1.0f - kEdgeShare), 0.0f, 1.0f);
  const float outerR = std::clamp((1.0f - right) / (1.0f - kEdgeShare), 0.0f, 1.0f);
  const float xl = kCentres[0] * outerL;
  const float xr = kCentres[1] + (kLastColumn - kCentres[1]) * outerR;
  return (left * xl + right * xr) / (left + right);
}
