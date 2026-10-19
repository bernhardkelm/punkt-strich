#include "touch/GestureLog.h"

#include <Arduino.h>

#include <cstring>

void GestureLog::record(const char* line) {
  char* slot = lines_[next_];
  strncpy(slot, line, kLineSize - 1);
  slot[kLineSize - 1] = '\0';
  next_ = static_cast<uint8_t>((next_ + 1) % kCount);
  Serial.println(slot);
}

void GestureLog::repeat(uint32_t nowMs) {
  if ((nowMs - repeatMs_) < kRepeatMs) {
    return;
  }

  repeatMs_ = nowMs;

  for (uint8_t i = 0; i < kCount; i++) {
    if (lines_[i][0] != '\0') {
      Serial.println(lines_[i]);
    }
  }
}
