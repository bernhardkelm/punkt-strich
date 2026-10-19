#pragma once

#include <cstdint>

namespace Board {

constexpr uint8_t kSdaPin = 8;
constexpr uint8_t kSclPin = 9;
constexpr uint8_t kMatrixCount = 2;
/** I2C addresses, ordered left to right */
inline constexpr uint8_t kMatrixAddresses[kMatrixCount] = {0x70, 0x71};
/** Canvas width in columns, 8 per backpack */
constexpr uint8_t kCanvasWidth = 16;
/** Column where the 8x8 animation viewport starts */
constexpr uint8_t kViewportX = 8;
/** Whether the canvas has a clock half next to the animation viewport */
constexpr bool kHasClockHalf = true;
/** Right coin under the top wall on touch channel T5, the left one is Touch::kLeftPin */
constexpr uint8_t kRightCoinPin = 5;
/** Whether USB VBUS reaches an ADC pin through a 100k/100k divider */
constexpr bool kHasVbusSense = true;
/** VBUS sense pin on ADC1, reads half of VBUS */
constexpr uint8_t kVbusSensePin = 3;
constexpr const char* kDefaultName = "strich";
constexpr uint8_t kDefaultBrightness = 5;
constexpr bool kDefaultAutoDim = true;
constexpr uint8_t kDefaultNightBrightness = 0;
constexpr uint16_t kDefaultDimTimeoutMin = 10;
/** Default clock face as its ClockFace enumerator value, 0 STAIRCASE, 1 TALL */
constexpr uint8_t kDefaultClockFace = 1;
constexpr uint8_t kDefaultTouchTrigger = 15;

}
