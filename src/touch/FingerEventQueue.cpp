#include "touch/FingerEventQueue.h"

void FingerEventQueue::push(const FingerEvent& event) {
  if (count_ >= kCapacity) {
    return;
  }

  items_[(head_ + count_) % kCapacity] = event;
  count_++;
}

FingerEvent FingerEventQueue::pop() {
  if (count_ == 0) {
    return FingerEvent{};
  }

  const FingerEvent event = items_[head_];
  head_ = static_cast<uint8_t>((head_ + 1) % kCapacity);
  count_--;
  return event;
}
