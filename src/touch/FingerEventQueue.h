#pragma once

#include "touch/FingerEvent.h"

#include <cstdint>

/** Holds the gestures one sample produced so they are handed out one per call, in order. */
class FingerEventQueue {
public:
  /**
   * Appends a gesture, dropped when the queue is full.
   * @param {const FingerEvent&} event - gesture to append
   * @returns {void}
   */
  void push(const FingerEvent& event);

  /**
   * Removes and returns the oldest gesture.
   * @returns {FingerEvent} - oldest gesture, NONE when the queue is empty
   */
  FingerEvent pop();

private:
  /** a sample yields at most two confirmations and one press gesture */
  static constexpr uint8_t kCapacity = 4;

  FingerEvent items_[kCapacity];
  uint8_t head_ = 0;
  uint8_t count_ = 0;
};
