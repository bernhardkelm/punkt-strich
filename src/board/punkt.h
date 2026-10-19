#pragma once

#include <cstdint>

namespace Board {

constexpr uint8_t kSdaPin = 8;
constexpr uint8_t kSclPin = 9;
constexpr uint8_t kMatrixCount = 1;
/** I2C addresses, ordered left to right */
inline constexpr uint8_t kMatrixAddresses[kMatrixCount] = {0x70};
/** Canvas width in columns, 8 per backpack */
constexpr uint8_t kCanvasWidth = 8;
/** Column where the 8x8 animation viewport starts */
constexpr uint8_t kViewportX = 0;
/** Whether the canvas has a clock half next to the animation viewport */
constexpr bool kHasClockHalf = false;
/** Right coin pin, read only behind kHasClockHalf, punkt has one coin and never begins it */
constexpr uint8_t kRightCoinPin = 5;
/** Whether USB VBUS reaches an ADC pin, punkt has no sense pin and never touches GPIO3 */
constexpr bool kHasVbusSense = false;
/** VBUS sense pin, unused while kHasVbusSense is false */
constexpr uint8_t kVbusSensePin = 0;
constexpr const char* kDefaultName = "punkt";
constexpr uint8_t kDefaultBrightness = 8;
constexpr bool kDefaultAutoDim = false;
constexpr uint8_t kDefaultNightBrightness = 2;
constexpr uint16_t kDefaultDimTimeoutMin = 5;
/** Default clock face as its ClockFace enumerator value, 0 STAIRCASE, 1 TALL */
constexpr uint8_t kDefaultClockFace = 0;
constexpr uint8_t kDefaultTouchTrigger = 25;

}
