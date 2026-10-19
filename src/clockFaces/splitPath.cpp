#include "clockFaces/splitPath.h"

#include "clockFaces/canvasFrame.h"
#include "clockFaces/digitPairs.h"
#include "clockFaces/frameUnion.h"
#include "clockFaces/minuteBar.h"
#include "clockFaces/minuteSquares.h"
#include "clockFaces/splitClockFrame.h"

namespace {

/** Wait before each row the Tall face loses on the hour and the minute pair */
constexpr uint32_t kSquashRowMs = 80;
/** Wait before the DIGITS minutes switch to the 3x3 font */
constexpr uint32_t kFontSwitchMs = 80;
/** Wait before each column the minutes slide left */
constexpr uint32_t kSlideColumnMs = 50;
/** Wait before each row the minute pair drops */
constexpr uint32_t kDropRowMs = 50;
/** Wait before each pixel a minute square gains */
constexpr uint32_t kSquareFillMs = 35;
/** Wait before each pixel the minute bar gains */
constexpr uint32_t kBarFillMs = 30;
/** Rows the minute pair drops until it left the canvas */
constexpr uint8_t kDropRows = kStaircaseRows;
/** Columns the 3x3 minutes slide from the full face's minute column to column 0 */
constexpr uint8_t kSmallSlideColumns = kFaceMinuteX;
constexpr uint8_t kTens = 10;
constexpr uint8_t kMaxRuns = 4;

/** What the frames of a run show, each step moves its value on by one */
enum class Stage : uint8_t {
  SQUASH,       // value: rows the digits lost from the Tall height
  SLIDE,        // value: columns the 3x4 minute pair moved left
  SMALL_SLIDE,  // value: columns the 3x3 minute pair moved left
  DROP,         // value: rows the 3x4 minute pair dropped
  TENS_FILL,    // value: lit pixels of the tens square, the ones square empty
  ONES_FILL,    // value: lit pixels of the ones square, the tens square full
  BAR_FILL      // value: lit pixels of the bar
};

/** Consecutive path steps of one stage at one pace */
struct Run {
  Stage stage;
  /** value of the run's first step */
  uint8_t first;
  uint8_t steps;
  uint32_t stepMs;
};

/** The runs a path takes after the full Tall face, in order */
struct Path {
  Run runs[kMaxRuns];
  uint8_t count;
};

/** Where a path position lies */
struct Spot {
  Stage stage;
  uint8_t value;
  uint32_t stepMs;
};

/**
 * Returns the runs of a style's path, every path starting with the squash.
 * @param {SplitClock} style - split clock style the path leads to
 * @param {uint8_t} minute - 0 to 59
 * @returns {Path} - runs in order
 */
Path pathOf(SplitClock style, uint8_t minute) {
  const Run squash{Stage::SQUASH, 1, kSquashSteps, kSquashRowMs};
  const Run drop{Stage::DROP, 1, kDropRows, kDropRowMs};
  const uint8_t tens = static_cast<uint8_t>(minute / kTens);
  const uint8_t ones = static_cast<uint8_t>(minute % kTens);

  switch (style) {
    case SplitClock::SQUARES:
      return {{squash, drop, {Stage::TENS_FILL, 1, tens, kSquareFillMs},
               {Stage::ONES_FILL, 1, ones, kSquareFillMs}}, 4};
    case SplitClock::STAGGER:
      return {{squash, {Stage::SLIDE, 1, kMinuteShiftMax, kSlideColumnMs}}, 2};
    case SplitClock::BAR:
      return {{squash, drop, {Stage::BAR_FILL, 1, minuteBarPixels(minute), kBarFillMs}}, 3};
    case SplitClock::DIGITS:
    default:
      return {{squash, {Stage::SMALL_SLIDE, 0, 1, kFontSwitchMs},
               {Stage::SMALL_SLIDE, 1, kSmallSlideColumns, kSlideColumnMs}}, 3};
  }
}

/**
 * Returns the stage, value and pace of a path position, position 0 being the full Tall face.
 * @param {const Path&} path - runs of the path
 * @param {uint8_t} position - 0 to the path end, a later one lands on the last step
 * @returns {Spot} - stage and value to draw, the wait before the step into the position
 */
Spot spotOf(const Path& path, uint8_t position) {
  Spot spot{Stage::SQUASH, 0, kSquashRowMs};
  uint8_t left = position;

  for (uint8_t i = 0; i < path.count && left > 0; i++) {
    const Run& run = path.runs[i];

    if (run.steps == 0) {
      continue;
    }

    const uint8_t taken = left < run.steps ? left : run.steps;
    spot = {run.stage, static_cast<uint8_t>(run.first + taken - 1), run.stepMs};
    left = static_cast<uint8_t>(left - taken);
  }

  return spot;
}

/**
 * Returns the minute pixels of a stage the path draws from frames, every stage but the squash and
 * the slide of the 3x4 pair.
 * @param {Stage} stage - SMALL_SLIDE, DROP, TENS_FILL, ONES_FILL or BAR_FILL
 * @param {uint8_t} minute - 0 to 59
 * @param {uint8_t} value - stage value of the step
 * @returns {Frame} - frame with the minute pixels set
 */
Frame minutesFrame(Stage stage, uint8_t minute, uint8_t value) {
  const int8_t offset = static_cast<int8_t>(value);

  switch (stage) {
    case Stage::SMALL_SLIDE:
      return smallPairFrame(minute, static_cast<int8_t>(kFaceMinuteX - offset), kSmallMinutesY);
    case Stage::DROP:
      return digitPairFrame(minute, kFaceMinuteX, static_cast<int8_t>(kStaircaseRows + offset));
    case Stage::TENS_FILL:
      return minuteSquaresFrame(value, 0);
    case Stage::ONES_FILL:
      return minuteSquaresFrame(static_cast<uint8_t>(minute / kTens), value);
    case Stage::BAR_FILL:
    default:
      return minuteBarFrame(value);
  }
}

}

uint8_t splitPathEnd(SplitClock style, uint8_t minute) {
  const Path path = pathOf(style, minute);
  uint8_t end = 0;

  for (uint8_t i = 0; i < path.count; i++) {
    end = static_cast<uint8_t>(end + path.runs[i].steps);
  }

  return end;
}

uint32_t splitPathStepMs(SplitClock style, uint8_t minute, uint8_t position) {
  return spotOf(pathOf(style, minute), position).stepMs;
}

void drawSplitPath(Display& display, SplitClock style, uint8_t hour, uint8_t minute,
                   uint8_t position) {
  const Spot spot = spotOf(pathOf(style, minute), position);

  if (spot.stage == Stage::SQUASH) {
    const uint8_t rows = static_cast<uint8_t>(kTallRows - spot.value);
    drawClockFrame(display, hour, minute, 0, rows, rows);
    return;
  }

  if (spot.stage == Stage::SLIDE) {
    drawClockFrame(display, hour, minute, spot.value, kStaircaseRows, kStaircaseRows);
    return;
  }

  const Frame minutes = minutesFrame(spot.stage, minute, spot.value);
  drawCanvasFrame(display, frameUnion(splitHoursFrame(hour), minutes));
}
