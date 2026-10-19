#include "state/State.h"

#include <cstring>

namespace {

constexpr const char* kAnonymous = "";

}

const char* StateMachine::animationNameAt(size_t index) const {
  if (index >= kAnimationCount) {
    return nullptr;
  }

  return all_[index]->name();
}

int StateMachine::animationIndex(const char* name) const {
  if (name == nullptr) {
    return -1;
  }

  for (size_t i = 0; i < kAnimationCount; i++) {
    if (strcmp(all_[i]->name(), name) == 0) {
      return static_cast<int>(i);
    }
  }

  return -1;
}

void StateMachine::preview(size_t index, uint32_t nowMs) {
  if (index >= kAnimationCount) {
    return;
  }

  preview_ = all_[index];
  previewStartedMs_ = nowMs;
  previewUntilMs_ = nowMs + kPreviewMs;
  previewFinishedMs_ = 0;
  lastActivityMs_ = nowMs;
  sweepPreview(nowMs);
  // a session id makes show() restart, so an animation asked for again plays from the start
  show(state_, kDefaultActivity, -1, kAnonymous, "preview");
}

void StateMachine::expirePreview(uint32_t nowMs) {
  if (preview_ == nullptr) {
    return;
  }

  sweepPreview(nowMs);

  // a one-shot a slide still holds back reports the end of its last run
  if (previewFinishedMs_ == 0 && !pictureHeld() && preview_->finished()) {
    previewFinishedMs_ = nowMs;
  }

  const bool over = static_cast<int32_t>(nowMs - previewUntilMs_) >= 0;
  const bool held = previewFinishedMs_ != 0 &&
                    static_cast<int32_t>(nowMs - previewFinishedMs_ - kPreviewHoldMs) >= 0;

  if (!over && !held) {
    return;
  }

  preview_ = nullptr;
  previewStartedMs_ = 0;
  previewUntilMs_ = 0;
  previewFinishedMs_ = 0;
  refresh(nullptr, "preview end");
}

void StateMachine::sweepPreview(uint32_t nowMs) {
  const bool sweeping = preview_ == &resetFill_ || preview_ == &otaSpiral_;

  // a running reset or OTA update writes its own progress, the preview must not overwrite it
  if (!sweeping || resetting_ || updating_) {
    return;
  }

  const uint32_t elapsed = nowMs - previewStartedMs_;
  const uint32_t pixels =
      elapsed >= kPreviewMs ? kPreviewPixels : elapsed * kPreviewPixels / kPreviewMs;

  if (preview_ == &resetFill_) {
    resetFill_.setProgress(static_cast<uint8_t>(pixels));
    return;
  }

  otaSpiral_.setProgress(static_cast<uint8_t>(pixels));
}
