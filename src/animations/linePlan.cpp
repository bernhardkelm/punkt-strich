#include "animations/linePlan.h"

#include <Arduino.h>

namespace {

constexpr uint8_t kMaxIndent = 2;
constexpr uint8_t kOpenerPercent = 35;
constexpr uint8_t kCloserPercent = 30;
constexpr uint8_t kBlankAfterCloserPercent = 50;
constexpr uint8_t kBlankPercent = 8;
constexpr uint8_t kGapPercent = 40;
constexpr uint8_t kShortLinePercent = 25;
constexpr uint8_t kFullLinePercent = 15;
constexpr uint8_t kCloserLength = 1;
constexpr uint8_t kMinLineLength = 2;
constexpr uint8_t kShortLineMax = 3;
constexpr uint8_t kMidLineMin = 3;
constexpr uint8_t kGapMinLength = 5;
constexpr uint8_t kGapEdgeMargin = 2;

}

LinePlan planLine(const LinePlan& previous, uint8_t indent, bool firstRow) {
  LinePlan plan;
  plan.indent = indent;

  const uint8_t blankPercent = previous.closer ? kBlankAfterCloserPercent : kBlankPercent;

  if (!firstRow && !previous.blank && !previous.opener && random(100) < blankPercent) {
    plan.blank = true;
    return plan;
  }

  if (indent > 0 && !previous.opener && random(100) < kCloserPercent) {
    plan.closer = true;
    plan.indent = static_cast<uint8_t>(indent - 1);
    plan.length = kCloserLength;
    return plan;
  }

  if (indent < kMaxIndent && random(100) < kOpenerPercent) {
    plan.opener = true;
  }

  const uint8_t width = static_cast<uint8_t>(Display::kWidth - plan.indent);
  const long roll = random(100);
  uint8_t length = width;

  if (roll < kShortLinePercent) {
    length = static_cast<uint8_t>(kMinLineLength + random(kShortLineMax - kMinLineLength + 1));
  } else if (roll < kShortLinePercent + kFullLinePercent || width <= kMidLineMin) {
    length = width;
  } else {
    length = static_cast<uint8_t>(kMidLineMin + random(width - kMidLineMin));
  }

  if (length < kMinLineLength) {
    length = kMinLineLength;
  }

  if (length > width) {
    length = width;
  }

  plan.length = length;

  if (length >= kGapMinLength && random(100) < kGapPercent) {
    const uint8_t slots = static_cast<uint8_t>(length - 2 * kGapEdgeMargin);
    plan.gapColumn = static_cast<int8_t>(plan.indent + kGapEdgeMargin + random(slots));
  }

  return plan;
}

uint8_t nextIndent(const LinePlan& plan) {
  return plan.opener ? static_cast<uint8_t>(plan.indent + 1) : plan.indent;
}
