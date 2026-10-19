// Drives one firmware picture on the host and prints every frame as "t brightness row0..row7".
#include "Animation.h"
#include "Display.h"

#include "animations/Attention.h"
#include "animations/Clock.h"
#include "animations/CodeLines.h"
#include "animations/Compacting.h"
#include "animations/Done.h"
#include "animations/Error.h"
#include "animations/Glitch.h"
#include "animations/Knight.h"
#include "animations/Life.h"
#include "animations/Limit.h"
#include "animations/MatrixRain.h"
#include "animations/Off.h"
#include "animations/Orbit.h"
#include "animations/OtaSpiral.h"
#include "animations/Pipes.h"
#include "animations/Pointer.h"
#include "animations/Pong.h"
#include "animations/ResetFill.h"
#include "animations/Scan.h"
#include "animations/Terminal.h"
#include "animations/Tetris.h"
#include "animations/Unread.h"
#include "animations/UsageView.h"
#include "animations/Waiting.h"
#include "animations/WifiLost.h"
#include "animations/WifiSearch.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>

namespace {

constexpr uint32_t kStepMs = 5;
constexpr uint8_t kDefaultBrightness = Display::kMaxBrightness;
constexpr uint32_t kPixelCount = Display::kWidth * Display::kHeight;
/** matches ResetHold::kResetMs minus kArmMs, the window the hold fills the matrix in */
constexpr uint32_t kResetFillMs = 8000;
/** the OTA spiral has no timing of its own, an image lands in roughly this long */
constexpr uint32_t kOtaFillMs = 6000;

uint32_t g_nowMs = 0;
uint32_t g_rng = 2463534242u;
uint8_t g_lastRows[Display::kHeight] = {};
uint8_t g_lastBrightness = 255;
bool g_anyFrame = false;

}

/**
 * Returns the host clock the animations run on.
 * @returns {uint32_t} - milliseconds since the render started
 */
uint32_t millis() {
  return g_nowMs;
}

/**
 * Returns a pseudo random number from a xorshift generator, the Arduino signature.
 * @param {long} max - exclusive upper bound
 * @returns {long} - 0 to max - 1, 0 when max is not positive
 */
long random(long max) {
  g_rng ^= g_rng << 13;
  g_rng ^= g_rng >> 17;
  g_rng ^= g_rng << 5;
  return max <= 0 ? 0 : static_cast<long>(g_rng % static_cast<uint32_t>(max));
}

/**
 * Returns a pseudo random number in a range, the Arduino signature.
 * @param {long} min - inclusive lower bound
 * @param {long} max - exclusive upper bound
 * @returns {long} - min to max - 1
 */
long random(long min, long max) {
  return min + random(max - min);
}

/**
 * Prints the frame buffer whenever it differs from the one before it.
 * @param {const Display&} display - matrix the animation just pushed
 * @returns {void}
 */
void simOnShow(const Display& display) {
  const bool same = g_anyFrame && display.brightness == g_lastBrightness &&
                    memcmp(g_lastRows, display.rows, Display::kHeight) == 0;

  if (same) {
    return;
  }

  memcpy(g_lastRows, display.rows, Display::kHeight);
  g_lastBrightness = display.brightness;
  g_anyFrame = true;
  printf("%u %u", g_nowMs, display.brightness);

  for (int y = 0; y < Display::kHeight; y++) {
    printf(" %u", display.rows[y]);
  }

  printf("\n");
}

namespace {

/** One renderable picture: the animation plus the value the firmware feeds it while it runs. */
struct Picture {
  std::unique_ptr<Animation> animation;
  void (*drive)(Animation&, uint32_t) = nullptr;
};

/**
 * Feeds the reset fill the pixel count the button hold would have reached.
 * @param {Animation&} animation - the ResetFill instance
 * @param {uint32_t} nowMs - milliseconds since the render started
 * @returns {void}
 */
void driveReset(Animation& animation, uint32_t nowMs) {
  const uint32_t pixels = nowMs >= kResetFillMs ? kPixelCount : nowMs * kPixelCount / kResetFillMs;
  static_cast<ResetFill&>(animation).setProgress(static_cast<uint8_t>(pixels));
}

/**
 * Feeds the OTA spiral the share of the image an update would have received.
 * @param {Animation&} animation - the OtaSpiral instance
 * @param {uint32_t} nowMs - milliseconds since the render started
 * @returns {void}
 */
void driveOta(Animation& animation, uint32_t nowMs) {
  const uint32_t pixels = nowMs >= kOtaFillMs ? kPixelCount : nowMs * kPixelCount / kOtaFillMs;
  static_cast<OtaSpiral&>(animation).setProgress(static_cast<uint8_t>(pixels));
}

/**
 * Builds the usage view with the share of the 5 hour limit the state machine would report.
 * @param {uint8_t} percent - 0 to 100 share of the limit spent
 * @returns {Picture} - the usage picture
 */
Picture makeUsage(uint8_t percent) {
  auto view = std::make_unique<UsageView>();
  view->setPercent(percent);
  return Picture{std::move(view), nullptr};
}

/**
 * Wraps an animation that needs nothing fed to it.
 * @returns {Picture} - the picture running that animation
 */
template <typename T>
Picture plain() {
  return Picture{std::make_unique<T>(), nullptr};
}

/**
 * Builds the picture the firmware shows under a name, the names the docs and the API use.
 * @param {const char*} name - picture name, for example clock, pong or wifi-search
 * @param {uint8_t} percent - context percent, or the usage percent for the usage view
 * @returns {Picture} - the picture, with a null animation for an unknown name
 */
Picture makePicture(const char* name, uint8_t percent) {
  if (strcmp(name, "off") == 0) {
    return plain<Off>();
  }

  if (strcmp(name, "loading") == 0 || strcmp(name, "orbit") == 0) {
    return plain<Orbit>();
  }

  if (strcmp(name, "attention") == 0) {
    return plain<Attention>();
  }

  if (strcmp(name, "done") == 0) {
    return plain<Done>();
  }

  if (strcmp(name, "error") == 0) {
    return plain<Error>();
  }

  if (strcmp(name, "waiting") == 0) {
    return plain<Waiting>();
  }

  if (strcmp(name, "compacting") == 0) {
    return plain<Compacting>();
  }

  if (strcmp(name, "limit") == 0) {
    return plain<Limit>();
  }

  if (strcmp(name, "unread") == 0) {
    return plain<Unread>();
  }

  if (strcmp(name, "glitch") == 0) {
    return plain<Glitch>();
  }

  if (strcmp(name, "clock") == 0) {
    return plain<Clock>();
  }

  if (strcmp(name, "tetris") == 0) {
    return plain<Tetris>();
  }

  if (strcmp(name, "life") == 0) {
    return plain<Life>();
  }

  if (strcmp(name, "knight") == 0) {
    return plain<Knight>();
  }

  if (strcmp(name, "code") == 0) {
    return plain<CodeLines>();
  }

  if (strcmp(name, "rain") == 0) {
    return plain<MatrixRain>();
  }

  if (strcmp(name, "scan") == 0) {
    return plain<Scan>();
  }

  if (strcmp(name, "pipes") == 0) {
    return plain<Pipes>();
  }

  if (strcmp(name, "pointer") == 0) {
    return plain<Pointer>();
  }

  if (strcmp(name, "pong") == 0) {
    return plain<Pong>();
  }

  if (strcmp(name, "terminal") == 0) {
    return plain<Terminal>();
  }

  if (strcmp(name, "reset") == 0) {
    return Picture{std::make_unique<ResetFill>(), driveReset};
  }

  if (strcmp(name, "wifi-search") == 0) {
    return plain<WifiSearch>();
  }

  if (strcmp(name, "wifi-lost") == 0) {
    return plain<WifiLost>();
  }

  if (strcmp(name, "ota") == 0) {
    return Picture{std::make_unique<OtaSpiral>(), driveOta};
  }

  if (strcmp(name, "usage") == 0) {
    return makeUsage(percent);
  }

  return Picture{};
}

}

/**
 * Renders one picture for a duration and prints its frames to stdout.
 * @param {int} argc - argument count
 * @param {char**} argv - name, durationMs, optional seed, percent and brightness ceiling
 * @returns {int} - 0 on success, 1 on a bad argument
 */
int main(int argc, char** argv) {
  if (argc < 3) {
    fprintf(stderr, "usage: render <picture> <durationMs> [seed] [percent] [brightness]\n");
    return 1;
  }

  const uint32_t duration = static_cast<uint32_t>(atoi(argv[2]));

  if (argc > 3) {
    g_rng = static_cast<uint32_t>(atoi(argv[3]));
  }

  const uint8_t percent = argc > 4 ? static_cast<uint8_t>(atoi(argv[4])) : 0;
  Picture picture = makePicture(argv[1], percent);

  if (!picture.animation) {
    fprintf(stderr, "unknown picture %s\n", argv[1]);
    return 1;
  }

  Display display;
  const uint8_t ceiling = argc > 5 ? static_cast<uint8_t>(atoi(argv[5])) : kDefaultBrightness;

  // the matrix keeps the level the firmware set at boot, the ceiling stands in for it here
  display.brightness = ceiling;
  picture.animation->setMaxBrightness(ceiling);
  picture.animation->setContextPercent(percent);
  g_nowMs = 0;

  if (picture.drive != nullptr) {
    picture.drive(*picture.animation, g_nowMs);
  }

  picture.animation->begin(display);

  for (g_nowMs = 1; g_nowMs <= duration; g_nowMs += kStepMs) {
    if (picture.drive != nullptr) {
      picture.drive(*picture.animation, g_nowMs);
    }

    picture.animation->tick(display, g_nowMs);
  }

  // the horizon line tells the GIF step how far the dump was observed, a still frame emits nothing
  printf("end %u\n", duration);
  return 0;
}
