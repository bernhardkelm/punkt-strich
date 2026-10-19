#include "clockFaces/smallDigit.h"

namespace {

constexpr uint8_t kDigitCount = 10;

constexpr uint8_t kFont[kDigitCount][kSmallDigitRows] = {
  {0b111, 0b101, 0b111},
  {0b110, 0b010, 0b111},
  {0b110, 0b010, 0b011},
  {0b111, 0b011, 0b111},
  {0b101, 0b111, 0b001},
  {0b011, 0b010, 0b110},
  {0b100, 0b111, 0b111},
  {0b111, 0b001, 0b001},
  {0b011, 0b111, 0b110},
  {0b111, 0b111, 0b001},
};

}

uint8_t smallDigitRow(uint8_t digit, uint8_t row) {
  if (digit >= kDigitCount || row >= kSmallDigitRows) {
    return 0;
  }

  return kFont[digit][row];
}
