#include "state/State.h"

uint8_t StateMachine::contextPercent() const {
  return context_.percent();
}

void StateMachine::setContextPercent(uint8_t percent, uint32_t nowMs) {
  context_.set(percent, nowMs);
  updateContextPush(nowMs);
}

bool StateMachine::hasContext() const {
  return context_.received();
}

uint32_t StateMachine::contextAgeMs(uint32_t nowMs) const {
  return context_.ageMs(nowMs);
}

bool StateMachine::contextStale(uint32_t nowMs) const {
  return context_.stale(nowMs, kContextStaleMs);
}

void StateMachine::updateContextPush(uint32_t nowMs) {
  const uint8_t pushed = context_.shown(nowMs, kContextStaleMs);

  if (pushed == pushedContext_) {
    return;
  }

  pushedContext_ = pushed;

  for (Animation* animation : all_) {
    animation->setContextPercent(pushed);
  }

  idleGauge_.setContextPercent(pushed);
}

uint8_t StateMachine::usagePercent() const {
  return usage_.percent();
}

void StateMachine::setUsagePercent(uint8_t percent, uint32_t nowMs) {
  usage_.set(percent, nowMs);
  updateUsagePush(nowMs);
}

bool StateMachine::hasUsage() const {
  return usage_.received();
}

uint32_t StateMachine::usageAgeMs(uint32_t nowMs) const {
  return usage_.ageMs(nowMs);
}

bool StateMachine::usageStale(uint32_t nowMs) const {
  return usage_.stale(nowMs, kUsageStaleMs);
}

void StateMachine::updateUsagePush(uint32_t nowMs) {
  usageView_.setPercent(usage_.shown(nowMs, kUsageStaleMs));
}
