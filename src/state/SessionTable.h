#pragma once

#include "settings/Settings.h"

#include <cstddef>
#include <cstdint>
#include <freertos/FreeRTOS.h>
#include <freertos/portmacro.h>

// opaque declaration, state/State.h defines PunktState and includes this header
enum class PunktState : uint8_t;

/** Longest session id the API accepts, the anonymous session uses the empty string. */
constexpr size_t kSessionIdMax = 32;

/** One Claude Code session, trivially copyable so the HTTP task can snapshot the table. */
struct Session {
  char id[kSessionIdMax + 1];
  PunktState state;
  /** Activity value the session last reported, kDefaultActivity while it reported none. */
  uint8_t activity;
  /** WorkingAnimation value overriding the activity mapping, -1 when the mapping applies. */
  int8_t animation;
  /** State a GLITCH returns to once SessionTable::kGlitchMs passed, IDLE outside a glitch. */
  PunktState resumeState;
  /** Activity value the resumed state carries, kDefaultActivity outside a glitch. */
  uint8_t resumeActivity;
  /** WorkingAnimation value the resumed state carries, -1 outside a glitch. */
  int8_t resumeAnimation;
  uint32_t lastApiMs;
  /** Sender clock in ms of the last applied request, 0 while none carried one. */
  uint64_t lastTs;

  /**
   * Puts the session into a state on the default activity, without an animation override and
   * with nothing to resume into.
   * @param {PunktState} newState - state the session falls into
   * @param {uint32_t} apiMs - millis() the watchdog counts from
   * @returns {void}
   */
  void resetTo(PunktState newState, uint32_t apiMs);
};

/**
 * Fixed size table of the sessions sharing the matrix, keyed by their id. The Arduino task owns
 * every write, the HTTP task only calls copyTo(). A spinlock keeps that snapshot apart from the
 * writes that move a whole entry, which a plain copy would catch half done.
 */
class SessionTable {
public:
  static constexpr size_t kMaxSessions = 8;
  /** Minutes without a request after which a busy session falls back to IDLE */
  static constexpr uint16_t kStuckResetMin = 10;
  /** Minutes without a request after which a named session is dropped */
  static constexpr uint16_t kExpiryMin = 60;
  /**
   * Milliseconds a GLITCH owns its session, the 600 ms burst plus the 1.5 s held X, after which
   * the session returns to the state it interrupted
   */
  static constexpr uint32_t kGlitchMs = 2100;

  /**
   * Returns the entry of a session.
   * @param {const char*} id - session id, empty string for the anonymous session
   * @returns {Session*} - entry or nullptr when the session is unknown
   */
  Session* find(const char* id);

  /**
   * Adds a session in its initial state, the id is truncated to kSessionIdMax characters.
   * @param {const char*} id - session id, empty string for the anonymous session
   * @param {PunktState} state - state the session starts in
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {Session*} - new entry or nullptr when the table is full
   */
  Session* add(const char* id, PunktState state, uint32_t nowMs);

  /**
   * Signs a session off, unknown ids are ignored.
   * @param {const char*} id - session id, empty string for the anonymous session
   * @returns {void}
   */
  void remove(const char* id);

  /**
   * Returns how many sessions are held.
   * @returns {size_t} - 0 to kMaxSessions
   */
  size_t count() const;

  /**
   * Returns how many sessions carry an id, the anonymous session is not counted.
   * @returns {size_t} - 0 to kMaxSessions
   */
  size_t namedCount() const;

  /**
   * Returns one entry, the index must be below count().
   * @param {size_t} index - entry to read
   * @returns {Session&} - entry owned by the table
   */
  Session& at(size_t index);

  /**
   * Returns the entry that owns the display, the highest priority state first and the most
   * recently updated session among equals.
   * @returns {int} - index of the entry or -1 when the table is empty
   */
  int displayIndex() const;

  /**
   * Drops sessions that went quiet after kExpiryMin and falls busy ones back to IDLE after
   * kStuckResetMin, both without touching the API timestamps. The anonymous session is never
   * dropped, it only falls back.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {bool} - true when at least one entry changed
   */
  bool expire(uint32_t nowMs);

  /**
   * Puts every GLITCH session whose kGlitchMs passed back into the state it interrupted, with the
   * activity and the animation override that state carried. The API timestamps are left alone, so
   * the watchdog keeps counting from the failure.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {bool} - true when at least one entry changed
   */
  bool resumeGlitched(uint32_t nowMs);

  /**
   * Puts every session back to IDLE and resets its watchdog.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void idleAll(uint32_t nowMs);

  /**
   * Resets the watchdog of every session without changing a state.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void touchAll(uint32_t nowMs);

  /**
   * Copies the table for the HTTP task, which only reads it. The lock keeps the snapshot out of
   * a running add() or removal. Writes through find() or at() take no lock, so a snapshot row
   * may mix old and new fields of one session.
   * @param {Session*} out - array receiving at most max entries
   * @param {size_t} max - capacity of out
   * @returns {size_t} - number of entries written
   */
  size_t copyTo(Session* out, size_t max) const;

private:
  static constexpr uint32_t kMsPerMinute = 60000;

  /**
   * Drops one entry, the last entry takes its slot. The caller holds the lock.
   * @param {size_t} index - entry to drop
   * @returns {void}
   */
  void removeAt(size_t index);

  Session sessions_[kMaxSessions] = {};
  size_t count_ = 0;
  /** guards count_ and the entries against the copyTo() of the HTTP task */
  mutable portMUX_TYPE lock_ = portMUX_INITIALIZER_UNLOCKED;
};
