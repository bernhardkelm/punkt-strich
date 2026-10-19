#include "state/SessionTable.h"

#include "state/State.h"
#include "state/statePriority.h"

#include <cstring>

void Session::resetTo(PunktState newState, uint32_t apiMs) {
  state = newState;
  activity = kDefaultActivity;
  animation = -1;
  resumeState = PunktState::IDLE;
  resumeActivity = kDefaultActivity;
  resumeAnimation = -1;
  lastApiMs = apiMs;
}

Session* SessionTable::find(const char* id) {
  if (id == nullptr) {
    return nullptr;
  }

  for (size_t i = 0; i < count_; i++) {
    if (strncmp(sessions_[i].id, id, kSessionIdMax) == 0) {
      return &sessions_[i];
    }
  }

  return nullptr;
}

Session* SessionTable::add(const char* id, PunktState state, uint32_t nowMs) {
  if (id == nullptr || count_ >= kMaxSessions) {
    return nullptr;
  }

  // the slot beyond count_ is filled first, a snapshot only reaches it once count_ is published
  Session& entry = sessions_[count_];
  entry = {};
  strncpy(entry.id, id, kSessionIdMax);
  entry.resetTo(state, nowMs);

  portENTER_CRITICAL(&lock_);
  count_++;
  portEXIT_CRITICAL(&lock_);
  return &entry;
}

void SessionTable::remove(const char* id) {
  const Session* entry = find(id);

  if (entry == nullptr) {
    return;
  }

  const size_t index = static_cast<size_t>(entry - sessions_);
  portENTER_CRITICAL(&lock_);
  removeAt(index);
  portEXIT_CRITICAL(&lock_);
}

size_t SessionTable::count() const {
  return count_;
}

size_t SessionTable::namedCount() const {
  size_t named = 0;

  for (size_t i = 0; i < count_; i++) {
    if (sessions_[i].id[0] != '\0') {
      named++;
    }
  }

  return named;
}

Session& SessionTable::at(size_t index) {
  return sessions_[index];
}

int SessionTable::displayIndex() const {
  int best = -1;
  uint8_t bestPriority = 0;

  for (size_t i = 0; i < count_; i++) {
    const uint8_t priority = statePriority(sessions_[i].state);

    if (best < 0 || priority > bestPriority) {
      best = static_cast<int>(i);
      bestPriority = priority;
      continue;
    }

    const uint32_t bestMs = sessions_[static_cast<size_t>(best)].lastApiMs;

    if (priority == bestPriority && static_cast<int32_t>(sessions_[i].lastApiMs - bestMs) > 0) {
      best = static_cast<int>(i);
    }
  }

  return best;
}

bool SessionTable::expire(uint32_t nowMs) {
  bool changed = false;

  portENTER_CRITICAL(&lock_);

  // walking backwards keeps the entry removeAt() moves into the freed slot out of the way
  for (size_t i = count_; i > 0; i--) {
    Session& entry = sessions_[i - 1];
    const uint32_t elapsedMin = (nowMs - entry.lastApiMs) / kMsPerMinute;

    // the anonymous entry is the board's own session, it rests on the clock instead of leaving
    if (elapsedMin >= kExpiryMin && entry.id[0] != '\0') {
      removeAt(i - 1);
      changed = true;
      continue;
    }

    if (elapsedMin >= kStuckResetMin && isBusyState(entry.state)) {
      // the fallback keeps its timestamp, so the session still expires kExpiryMin after the request
      entry.resetTo(PunktState::IDLE, entry.lastApiMs);
      changed = true;
    }
  }

  portEXIT_CRITICAL(&lock_);
  return changed;
}

bool SessionTable::resumeGlitched(uint32_t nowMs) {
  bool changed = false;

  portENTER_CRITICAL(&lock_);

  for (size_t i = 0; i < count_; i++) {
    Session& entry = sessions_[i];

    if (entry.state != PunktState::GLITCH) {
      continue;
    }

    // a nowMs sampled before the request must read as not elapsed, so the gap is signed
    if (static_cast<int32_t>(nowMs - entry.lastApiMs) < static_cast<int32_t>(kGlitchMs)) {
      continue;
    }

    entry.state = entry.resumeState;
    entry.activity = entry.resumeActivity;
    entry.animation = entry.resumeAnimation;
    entry.resumeState = PunktState::IDLE;
    entry.resumeActivity = kDefaultActivity;
    entry.resumeAnimation = -1;
    changed = true;
  }

  portEXIT_CRITICAL(&lock_);
  return changed;
}

void SessionTable::idleAll(uint32_t nowMs) {
  portENTER_CRITICAL(&lock_);

  for (size_t i = 0; i < count_; i++) {
    sessions_[i].resetTo(PunktState::IDLE, nowMs);
  }

  portEXIT_CRITICAL(&lock_);
}

void SessionTable::touchAll(uint32_t nowMs) {
  portENTER_CRITICAL(&lock_);

  for (size_t i = 0; i < count_; i++) {
    sessions_[i].lastApiMs = nowMs;
  }

  portEXIT_CRITICAL(&lock_);
}

size_t SessionTable::copyTo(Session* out, size_t max) const {
  portENTER_CRITICAL(&lock_);
  const size_t total = count_ < max ? count_ : max;

  for (size_t i = 0; i < total; i++) {
    out[i] = sessions_[i];
  }

  portEXIT_CRITICAL(&lock_);
  return total;
}

void SessionTable::removeAt(size_t index) {
  if (index >= count_) {
    return;
  }

  count_--;
  sessions_[index] = sessions_[count_];
}
