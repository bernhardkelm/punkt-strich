// Drives one strich picture on the host and prints its frames, 16 bit rows after a width line.
#include "Animation.h"
#include "Display.h"
#include "SlideRoundTrip.h"
#include "SplitPicture.h"
#include "hostServices.h"

#include "animations/Knight.h"
#include "animations/UsageView.h"
#include "clockFaces/BatteryFace.h"
#include "clockFaces/WideClock.h"
#include "settings/Settings.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>

namespace {

constexpr uint32_t kStepMs = 5;
constexpr uint8_t kRenderBrightness = Display::kMaxBrightness;

uint32_t g_nowMs = 0;
Settings g_settings;

}

/**
 * Returns the host clock the pictures run on.
 * @returns {uint32_t} - milliseconds since the render started
 */
uint32_t millis() {
  return g_nowMs;
}

namespace {

/**
 * Builds the full canvas clock in a face.
 * @param {ClockFace} face - clock face to show
 * @param {bool} lowBattery - true adds the blinking low-battery dot
 * @returns {std::unique_ptr<Animation>} - the clock picture
 */
std::unique_ptr<Animation> makeWideClock(ClockFace face, bool lowBattery) {
  g_settings.setClockFace(face);
  auto clock = std::make_unique<WideClock>(g_settings);
  clock->setLowBattery(lowBattery);
  return clock;
}

/**
 * Builds the split picture of a split clock style with the knight beside it.
 * @param {SplitClock} style - split clock style
 * @returns {std::unique_ptr<Animation>} - the split picture
 */
std::unique_ptr<Animation> makeSplit(SplitClock style) {
  g_settings.setSplitClock(style);
  return std::make_unique<SplitPicture>(g_settings, std::make_unique<Knight>());
}

/**
 * Builds the split picture with the usage view beside the DIGITS split clock.
 * @param {uint8_t} percent - 0 to 100 share of the limit spent
 * @returns {std::unique_ptr<Animation>} - the split picture
 */
std::unique_ptr<Animation> makeUsage(uint8_t percent) {
  auto view = std::make_unique<UsageView>();
  view->setPercent(percent);
  g_settings.setSplitClock(SplitClock::DIGITS);
  return std::make_unique<SplitPicture>(g_settings, std::move(view));
}

/**
 * Builds the battery view for a power state.
 * @param {bool} usb - true shows the board on USB power
 * @param {uint8_t} percent - charge estimate on battery
 * @returns {std::unique_ptr<Animation>} - the battery picture
 */
std::unique_ptr<Animation> makeBattery(bool usb, uint8_t percent) {
  setHostBattery(usb, percent);
  return std::make_unique<BatteryFace>();
}

/**
 * Builds the round trip from the Tall face into the DIGITS split picture with the knight and back.
 * @returns {std::unique_ptr<Animation>} - the slide picture
 */
std::unique_ptr<Animation> makeSlide() {
  g_settings.setClockFace(ClockFace::TALL);
  g_settings.setSplitClock(SplitClock::DIGITS);
  return std::make_unique<SlideRoundTrip>(g_settings, std::make_unique<Knight>());
}

/**
 * Builds a strich picture by the name the docs use.
 * @param {const char*} name - picture name, for example normal, slide or split-bar
 * @param {uint8_t} percent - usage percent for usage, charge percent for battery
 * @returns {std::unique_ptr<Animation>} - the picture, null for an unknown name
 */
std::unique_ptr<Animation> makePicture(const char* name, uint8_t percent) {
  if (strcmp(name, "clock-staircase") == 0) {
    return makeWideClock(ClockFace::STAIRCASE, false);
  }

  if (strcmp(name, "clock-tall") == 0) {
    return makeWideClock(ClockFace::TALL, false);
  }

  if (strcmp(name, "low-battery") == 0) {
    return makeWideClock(ClockFace::TALL, true);
  }

  if (strcmp(name, "normal") == 0 || strcmp(name, "split-digits") == 0) {
    return makeSplit(SplitClock::DIGITS);
  }

  if (strcmp(name, "split-squares") == 0) {
    return makeSplit(SplitClock::SQUARES);
  }

  if (strcmp(name, "split-stagger") == 0) {
    return makeSplit(SplitClock::STAGGER);
  }

  if (strcmp(name, "split-bar") == 0) {
    return makeSplit(SplitClock::BAR);
  }

  if (strcmp(name, "usage") == 0) {
    return makeUsage(percent);
  }

  if (strcmp(name, "battery") == 0) {
    return makeBattery(false, percent);
  }

  if (strcmp(name, "battery-charging") == 0) {
    return makeBattery(true, percent);
  }

  if (strcmp(name, "slide") == 0) {
    return makeSlide();
  }

  return nullptr;
}

}

/**
 * Renders one strich picture for a duration and prints its frames to stdout.
 * @param {int} argc - argument count
 * @param {char**} argv - name, durationMs, seed (unused) and percent
 * @returns {int} - 0 on success, 1 on a bad argument
 */
int main(int argc, char** argv) {
  if (argc < 3) {
    fprintf(stderr, "usage: render-strich <picture> <durationMs> [seed] [percent]\n");
    return 1;
  }

  const uint32_t duration = static_cast<uint32_t>(atoi(argv[2]));
  const uint8_t percent = argc > 4 ? static_cast<uint8_t>(atoi(argv[4])) : 0;
  std::unique_ptr<Animation> picture = makePicture(argv[1], percent);

  if (!picture) {
    fprintf(stderr, "unknown picture %s\n", argv[1]);
    return 1;
  }

  Display display;
  display.setBrightnessNow(kRenderBrightness);
  picture->setMaxBrightness(kRenderBrightness);
  printf("width %u\n", static_cast<unsigned>(Display::kCanvasWidth));
  picture->begin(display);

  for (g_nowMs = 1; g_nowMs <= duration; g_nowMs += kStepMs) {
    picture->tick(display, g_nowMs);
    display.tick(g_nowMs);
  }

  printf("end %u\n", duration);
  return 0;
}
