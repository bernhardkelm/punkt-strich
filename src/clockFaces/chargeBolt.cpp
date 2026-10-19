#include "clockFaces/chargeBolt.h"

#include "clockFaces/batteryOutline.h"
#include "clockFaces/glyphFrame.h"

#include <cstdint>

namespace {

constexpr uint8_t kBoltCols = 5;
constexpr uint8_t kBoltRows = 6;
constexpr int8_t kLeft = kBatteryInsideLeft + (kBatteryInsideWidth - kBoltCols) / 2;
constexpr int8_t kTop = kBatteryInsideTop + (kBatteryInsideHeight - kBoltRows) / 2;

constexpr uint8_t kBolt[kBoltRows] = {
  0b00010,
  0b00110,
  0b01111,
  0b11110,
  0b01100,
  0b01000,
};

}

Frame chargeBolt() {
  return glyphFrame(kBolt, kBoltRows, kBoltCols, kLeft, kTop);
}
