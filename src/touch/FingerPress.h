#pragma once

#include "touch/Coin.h"

#include <cstddef>
#include <cstdint>

/**
 * Metrics and rules of one press, from the finger coming near until it is gone. The start coin
 * and start column are fixed once the press reaches kStartShare, a weak early signal mislocates
 * the finger. A tap or a hold needs the start fixed, contact and the other coin staying below
 * kOtherShare. A swipe needs the other coin to have reached kOtherShare, the finger more than
 * kSwipeCross past the start coin centre towards it and kSwipeTravel of travel, since pressing
 * harder alone moves the estimate up to 3.5 columns.
 */
class FingerPress {
public:
  /**
   * Starts a press at the stronger coin, the start stays provisional until it is fixed.
   * @param {float} x - filtered finger column
   * @param {float} left - strength of the left coin
   * @param {float} right - strength of the right coin
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void begin(float x, float left, float right, uint32_t nowMs);

  /**
   * Feeds one sample of a press: tracks the peak, fixes the start at kStartShare and from then
   * on tracks the other coin's peak and the column range.
   * @param {float} x - filtered finger column
   * @param {float} left - strength of the left coin
   * @param {float} right - strength of the right coin
   * @returns {void}
   */
  void track(float x, float left, float right);

  /**
   * Widens the column range to a sample of a running swipe.
   * @param {float} x - filtered finger column
   * @returns {void}
   */
  void widen(float x);

  /**
   * Returns whether the start coin and column are fixed.
   * @returns {bool} - true once the press reached kStartShare
   */
  bool fixed() const;

  /**
   * Returns the start coin, provisional until fixed().
   * @returns {Coin} - coin the press started on
   */
  Coin coin() const;

  /**
   * Returns when the press started.
   * @returns {uint32_t} - millis() of the press start
   */
  uint32_t startMs() const;

  /**
   * Returns how long the press lasts.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {uint32_t} - milliseconds since the press start
   */
  uint32_t lengthMs(uint32_t nowMs) const;

  /**
   * Returns whether the press is a contact on one coin: start fixed, peak at kTapContact, other
   * coin below kOtherShare.
   * @returns {bool} - true while the press qualifies as a tap or a hold apart from its length
   */
  bool singleContact() const;

  /**
   * Returns whether the finger stayed near the start column, a lifting tap does, a swipe does not.
   * @param {float} x - filtered finger column
   * @returns {bool} - true while the finger is within kStayColumns of the start column
   */
  bool stayed(float x) const;

  /**
   * Returns whether the press is a tap if released now.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {bool} - true for a single contact of at most kTapMaxMs
   */
  bool tap(uint32_t nowMs) const;

  /**
   * Returns whether the press is a hold if released now.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {bool} - true for a single contact of at least kHoldMs
   */
  bool hold(uint32_t nowMs) const;

  /**
   * Returns whether the press turns into a swipe at the given column.
   * @param {float} x - filtered finger column
   * @returns {bool} - true once the swipe rules are met
   */
  bool swipeReady(float x) const;

  /**
   * Returns whether a swipe released at the given column completes.
   * @param {float} x - filtered finger column
   * @returns {bool} - true at least kCompletePastMiddle past the canvas middle on the far side
   */
  bool completes(float x) const;

  /**
   * Writes the CSV summary line of the press.
   * @param {char*} line - buffer to write into
   * @param {size_t} size - buffer size including the terminator
   * @param {const char*} kind - gesture name, tap, hold, swipe, none and so on
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void summarize(char* line, size_t size, const char* kind, uint32_t nowMs) const;

private:
  /** strength at which the start coin and start column are fixed */
  static constexpr float kStartShare = 0.35f;
  /** strength the other coin must reach before a press can become a swipe */
  static constexpr float kOtherShare = 0.35f;
  /** share of its press peak the start coin must fall below, a second finger keeps it at peak */
  static constexpr float kHandoverShare = 0.85f;
  /** peak strength a tap or a hold must reach */
  static constexpr float kTapContact = 0.45f;
  static constexpr uint32_t kTapMaxMs = 700;
  /** columns a press may drift and still end on contact loss, a swipe dips in the coin gap */
  static constexpr float kStayColumns = 2.0f;
  static constexpr uint32_t kHoldMs = 800;
  /** columns a press must travel before it turns into a swipe */
  static constexpr float kSwipeTravel = 2.5f;
  /** columns past the start coin centre the finger must reach for a swipe */
  static constexpr float kSwipeCross = 2.0f;
  /** columns past the canvas middle a released swipe must have reached to complete */
  static constexpr float kCompletePastMiddle = 0.5f;
  static constexpr float kMiddle = 7.5f;

  float startX_ = 0.0f;
  float minX_ = 0.0f;
  float maxX_ = 0.0f;
  /** highest strength of either coin during the press */
  float pressMax_ = 0.0f;
  /** highest strength of the other coin since the start was fixed */
  float otherMax_ = 0.0f;
  float startMax_ = 0.0f;
  float startNow_ = 0.0f;
  uint32_t startMs_ = 0;
  Coin coin_ = Coin::LEFT;
  bool fixed_ = false;
};
