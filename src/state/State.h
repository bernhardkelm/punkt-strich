#pragma once

#include "Animation.h"
#include "Display.h"
#include "animations/Attention.h"
#include "animations/Clock.h"
#include "animations/CodeLines.h"
#include "animations/Compacting.h"
#include "animations/Done.h"
#include "animations/Error.h"
#include "animations/Glitch.h"
#include "animations/IdleGauge.h"
#include "animations/Knight.h"
#include "animations/Life.h"
#include "animations/Limit.h"
#include "animations/MatrixRain.h"
#include "animations/Off.h"
#include "animations/Orbit.h"
#include "animations/OtaSpiral.h"
#include "animations/Pipes.h"
#include "animations/Pointer.h"
#include "animations/Pong.h"
#include "animations/ResetFill.h"
#include "animations/Scan.h"
#include "animations/Terminal.h"
#include "animations/Tetris.h"
#include "animations/Unread.h"
#include "animations/UsageView.h"
#include "animations/Waiting.h"
#include "animations/WifiLost.h"
#include "animations/WifiSearch.h"
#include "board/boardConfig.h"
#include "clockFaces/BatteryFace.h"
#include "clockFaces/CanvasWipe.h"
#include "clockFaces/LayoutSlide.h"
#include "clockFaces/SideClock.h"
#include "clockFaces/WideClock.h"
#include "network/WifiSetup.h"
#include "settings/Settings.h"
#include "state/ReportedPercent.h"
#include "state/SessionTable.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <ctime>

/** The twelve states punkt can show, the value is what the API and Commands carry. */
enum class PunktState : uint8_t {
  OFF,
  LOADING,
  IDLE,
  WORKING,
  ATTENTION,
  DONE,
  ERROR,
  WAITING,
  COMPACTING,
  LIMIT,
  UNREAD,
  GLITCH
};

/**
 * What the matrix shows besides the states that break through, CLOCK is the boot default. BATTERY
 * only exists where StateMachine::kHasBatteryView holds.
 */
enum class View : uint8_t { CLOCK, NORMAL, USAGE, OFF, BATTERY };

/** Why the matrix is dark, TIMEOUT and SCHEDULE are the offs the board leaves on its own. */
enum class OffReason : uint8_t { NONE, MANUAL, TIMEOUT, SCHEDULE };

/**
 * Collects every animation the state machine owns into a list whose length the arguments deduce,
 * so a list that no longer matches StateMachine::kAnimationCount fails to compile instead of
 * leaving a null slot behind.
 * @param {Ts*...} animations - one pointer per animation, in the order they are listed
 * @returns {std::array<Animation*, sizeof...(Ts)>} - list of the animations
 */
template <typename... Ts>
constexpr std::array<Animation*, sizeof...(Ts)> animationList(Ts*... animations) {
  return {{animations...}};
}

/**
 * Owns every animation, picks the session that owns the matrix, falls back to the clock and
 * darkens the matrix once the timeouts of the display cascade run out.
 */
class StateMachine {
public:
  static constexpr size_t kAnimationCount = 26;
  static constexpr uint32_t kPreviewMs = 10000;
  /** Whether the board offers View::BATTERY, it needs the full canvas and the VBUS sense pin */
  static constexpr bool kHasBatteryView = Board::kHasClockHalf && Board::kHasVbusSense;

  StateMachine(Display& display, Settings& settings);

  /** Enters LOADING, the first Wi-Fi connection or API call leaves it again. */
  void begin(uint32_t nowMs);

  /**
   * Switches state on behalf of the board itself, LOADING latches and overrides every session
   * until the next state request, any other state is applied to the anonymous session.
   * @param {PunktState} state - state to enter
   * @param {const char*} source - what triggered the change, logged in brackets
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void set(PunktState state, const char* source, uint32_t nowMs);

  /**
   * Applies a state request to one session and resets that session's watchdog, LOADING included,
   * which sits in the table like any other state instead of latching. A request whose
   * sender timestamp is older than the last one applied to that session was overtaken on the way
   * and is dropped, unless the gap exceeds kReorderWindowMs, which only a jumping sender clock
   * explains.
   * @param {const char*} session - session id, empty string for the anonymous session
   * @param {PunktState} state - state to enter, OFF signs the session off
   * @param {uint8_t} activity - Activity value, kDefaultActivity when the request named none
   * @param {int8_t} animation - WorkingAnimation override, -1 when the activity mapping applies
   * @param {uint64_t} ts - sender clock in ms, 0 applies the request unconditionally
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void setFromApi(const char* session, PunktState state, uint8_t activity, int8_t animation,
                  uint64_t ts, uint32_t nowMs);

  /**
   * Leaves LOADING without a state request and shows the session table again, a failed OTA
   * update uses it. Does nothing while LOADING is not shown.
   * @param {const char*} source - what triggered the change, logged in brackets
   * @returns {void}
   */
  void endLoading(const char* source);

  /**
   * Sets the session that owns the matrix to IDLE, the button acknowledges a state with it.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void dismissDisplayed(uint32_t nowMs);

  /**
   * Answers an ATTENTION on the session that owns the matrix by putting it back to WORKING,
   * every other state is left alone.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void acknowledgeDisplayed(uint32_t nowMs);

  /**
   * Puts every session back to IDLE and ends LOADING, the anonymous session is created when the
   * table is empty.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void forceIdle(uint32_t nowMs);

  /**
   * Resets the watchdog of every session without changing a state and counts as activity, so a
   * touch or the button rewind the display cascade to its first stage.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void noteActivity(uint32_t nowMs);

  /**
   * Returns the state currently shown.
   * @returns {PunktState} - displayed state
   */
  PunktState current() const;

  /**
   * Copies the session table for the HTTP task, which only reads it.
   * @param {Session*} out - array receiving at most max entries
   * @param {size_t} max - capacity of out
   * @returns {size_t} - number of entries written
   */
  size_t sessions(Session* out, size_t max) const;

  /**
   * Returns how many sessions carry a session id, the anonymous session is not counted.
   * @returns {size_t} - 0 to SessionTable::kMaxSessions
   */
  size_t namedSessionCount() const;

  /**
   * Returns whether the board's own LOADING is latched, which overrides every session. A LOADING
   * a session requested does not count.
   * @returns {bool} - true from set(LOADING) until a state request, forceIdle() or endLoading()
   */
  bool loading() const;

  /**
   * Returns whether a system picture (reset fill, OTA spiral, Wi-Fi picture) owns the matrix.
   * @returns {bool} - true while resetting, updating, or the link is PORTAL or LOST
   */
  bool systemPicture() const;

  /**
   * Returns the context window fill last reported, without judging its age.
   * @returns {uint8_t} - 0 to 100, 0 when nothing was reported
   */
  uint8_t contextPercent() const;

  /**
   * Stores the context window fill with its timestamp and pushes it into every animation.
   * @param {uint8_t} percent - 0 hides the gauge, 1 to 100 fill it
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void setContextPercent(uint8_t percent, uint32_t nowMs);

  /**
   * Returns whether a context window fill was ever reported.
   * @returns {bool} - true after the first report, also once it went stale
   */
  bool hasContext() const;

  /**
   * Returns how long ago the context window fill was reported.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {uint32_t} - milliseconds since the report, 0 when nothing was reported
   */
  uint32_t contextAgeMs(uint32_t nowMs) const;

  /**
   * Returns whether the context window fill is too old to show.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {bool} - true when nothing was reported or the report is older than 10 minutes
   */
  bool contextStale(uint32_t nowMs) const;

  /**
   * Returns the share of the 5 hour usage limit last reported, without judging its age.
   * @returns {uint8_t} - 0 to 100, 0 when nothing was reported
   */
  uint8_t usagePercent() const;

  /**
   * Stores the usage share with its timestamp and pushes it into the usage view.
   * @param {uint8_t} percent - 0 shows the placeholder pixel, 1 to 100 fill the view
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void setUsagePercent(uint8_t percent, uint32_t nowMs);

  /**
   * Returns whether a usage share was ever reported.
   * @returns {bool} - true after the first report, also once it went stale
   */
  bool hasUsage() const;

  /**
   * Returns how long ago the usage share was reported.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {uint32_t} - milliseconds since the report, 0 when nothing was reported
   */
  uint32_t usageAgeMs(uint32_t nowMs) const;

  /**
   * Returns whether the usage share is too old to show.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {bool} - true when nothing was reported or the report is older than an hour
   */
  bool usageStale(uint32_t nowMs) const;

  /**
   * Returns the name of the animation on the matrix, which the view may have chosen instead of
   * the state's own.
   * @returns {const char*} - animation name as logged in the switch line
   */
  const char* animationName() const;

  /**
   * Returns the name of one of the animations this machine owns.
   * @param {size_t} index - position in the animation list, below kAnimationCount
   * @returns {const char*} - animation name, nullptr when the index is out of range
   */
  const char* animationNameAt(size_t index) const;

  /**
   * Returns the position of an animation in the list.
   * @param {const char*} name - animation name to look up, may be null
   * @returns {int} - index below kAnimationCount, -1 when the name is unknown
   */
  int animationIndex(const char* name) const;

  /**
   * Shows one animation over the view and the session state for kPreviewMs, the OFF view
   * included. Only the reset fill, the link pictures and the OTA spiral stay above it, the
   * picture from before comes back on its own. Counts as activity, and an animation already
   * being previewed plays from the start again.
   * @param {size_t} index - position in the animation list, an index out of range is ignored
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void preview(size_t index, uint32_t nowMs);

  /**
   * Returns whether the matrix shows the clock face, which is what resting is measured on. The side
   * clock of a split picture does not count.
   * @returns {bool} - true while the clock or the full canvas clock face runs
   */
  bool showsClock() const;

  /**
   * Returns when the matrix last changed or the user last acted, whichever is later, so the
   * stages of the display cascade all count from the same moment.
   * @returns {uint32_t} - millis() the current rest started at
   */
  uint32_t restingSinceMs() const;

  /**
   * Returns when the user last acted, an API request, a touch, the button, a view change or a
   * preview.
   * @returns {uint32_t} - millis() of the last activity
   */
  uint32_t lastActivityMs() const;

  /**
   * Returns which view the matrix shows.
   * @returns {View} - CLOCK after boot, the view is not persisted
   */
  View view() const;

  /**
   * Switches the view on behalf of the user and counts as activity, a repeated view is ignored.
   * Entering and leaving OFF is always accepted and remembers the view OFF was entered from,
   * entering it is a manual off. Entering BATTERY and leaving it for CLOCK are accepted as well,
   * every other change needs a named session: without one the view stays and the glitch flashes
   * for kNoSessionFlashMs instead.
   * @param {View} view - view to enter
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void setView(View view, uint32_t nowMs);

  /**
   * Returns whether setView() would accept the view right now, a repeated view counts as accepted.
   * @param {View} view - view to enter
   * @returns {bool} - false when the change needs a named session and there is none
   */
  bool viewAllowed(View view) const;

  /**
   * Returns the view toggleView() would enter.
   * @returns {View} - next view of the cycle, CLOCK from BATTERY, the current view while it is OFF
   */
  View nextView() const;

  /**
   * Cycles CLOCK, NORMAL, USAGE and back to CLOCK, BATTERY returns to CLOCK, OFF is left alone
   * and is not part of the cycle.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void toggleView(uint32_t nowMs);

  /**
   * Darkens the matrix as a view, or lights it again with the view it had before. Darkening this
   * way is a manual off, which only the user takes back.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void toggleOff(uint32_t nowMs);

  /**
   * Returns why the matrix is dark.
   * @returns {OffReason} - NONE while the view is not OFF
   */
  OffReason offReason() const;

  /**
   * Darkens the matrix on the board's own decision, remembering the view to come back to. Does
   * not count as activity, and a view that is already OFF keeps the reason it carries.
   * @param {OffReason} reason - TIMEOUT or SCHEDULE, a manual reason is ignored
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void enterAutoOff(OffReason reason, uint32_t nowMs);

  /**
   * Lights the matrix again with the view it was darkened from, as long as the board darkened it
   * itself. Does not count as activity, the caller decides whether its trigger does.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {bool} - true when an automatic off ended, false for NONE and MANUAL
   */
  bool leaveAutoOff(uint32_t nowMs);

  /**
   * Shows the factory reset fill over everything else, or takes it away again.
   * @param {int16_t} pixels - 0 to the canvas pixel count, -1 hands the matrix back to the view
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void setResetProgress(int16_t pixels, uint32_t nowMs);

  /**
   * Shows the OTA progress spiral over everything but the reset fill and the link pictures, or
   * takes it away again.
   * @param {int16_t} pixels - 0 to the canvas pixel count, -1 hands the matrix back to the view
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void setOtaProgress(int16_t pixels, uint32_t nowMs);

  /**
   * Stores what the station link is doing and swaps the picture when it changed. The link is a
   * display decision, not a state: nothing is persisted and the picture from before comes back
   * once the link is up again.
   * @param {WifiSetup::LinkState} link - what the station link is doing
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void setLink(WifiSetup::LinkState link, uint32_t nowMs);

  /**
   * Runs the watchdog, ticks the animation and the side clock of a split picture, or a running
   * layout slide in their place, and a running strip wipe, ends a done once its burst is over and
   * a glitch once its time is up, ends a preview and applies the clock, battery and off timeouts.
   * The board rests on the clock when the watchdog dropped the last named session.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void tick(uint32_t nowMs);

  /**
   * Sets the brightness ceiling every animation scales against and lets the running animation
   * ramp the display to its level under it, without waiting for a frame. Every animation reads the
   * ceiling again when it draws its next frame, so no redraw is forced here.
   * @param {uint8_t} level - 0 to 15
   * @param {uint32_t} fadeMs - how long the ramp takes, a slow one glides into the dimmed level
   * @returns {void}
   */
  void setBrightness(uint8_t level, uint32_t fadeMs = Display::kFadeMs);

  /**
   * Returns the brightness ceiling in effect, 0 to 15.
   * @returns {uint8_t} - ceiling the animations and the display run at
   */
  uint8_t brightness() const;

  /**
   * Stores whether the matrix runs on the dimmed ceiling, the status body reports it.
   * @param {bool} dimmed - true while the dimmed ceiling is in effect
   * @returns {void}
   */
  void setDimmed(bool dimmed);

  /**
   * Returns whether the matrix runs on the dimmed ceiling.
   * @returns {bool} - true while the dimmed ceiling is in effect
   */
  bool dimmed() const;

  /**
   * Re-selects the working animation after the activity mapping changed, the brightness ceiling
   * is handed in through setBrightness() separately.
   * @returns {void}
   */
  void applySettings();

private:
  static constexpr uint32_t kContextStaleMs = 600000;
  static constexpr uint32_t kUsageStaleMs = 3600000;
  static constexpr uint32_t kNoSessionFlashMs = 1500;
  /** How long the battery view stays without activity before it returns to the clock */
  static constexpr uint32_t kBatteryViewMs = 30000;
  /** Longest a nowMs sample may lag a timestamp taken after it, more counts as elapsed time */
  static constexpr uint32_t kSampleLagMaxMs = 1000;
  static constexpr uint64_t kReorderWindowMs = 10000;
  /** How long a previewed one-shot holds its end frame before the preview ends */
  static constexpr uint32_t kPreviewHoldMs = 1000;
  /** Pixels a previewed reset fill or OTA spiral sweeps to over kPreviewMs, the whole canvas */
  static constexpr uint32_t kPreviewPixels = Display::kCanvasWidth * Display::kHeight;

  /** What displayAnimationFor() puts on the matrix. */
  struct Picture {
    Animation& animation;
    /** the side clock keeps the left half while the animation runs in the viewport */
    bool split;
  };

  /**
   * Writes a state request into the session table and refreshes the display. A request that
   * came over the API, from any session, counts as activity, ends an automatic off and pulls the
   * CLOCK, USAGE and BATTERY view over to NORMAL, so the matrix shows what was posted. A manual
   * off stays dark, and the button and touch paths never move the view.
   * @param {const char*} session - session id, empty string for the anonymous session
   * @param {PunktState} state - state to enter, OFF signs the session off
   * @param {uint8_t} activity - Activity value, kDefaultActivity when the request named none
   * @param {int8_t} animation - WorkingAnimation override, -1 when the activity mapping applies
   * @param {const char*} source - what triggered the change, logged in brackets
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void request(const char* session, PunktState state, uint8_t activity, int8_t animation,
               const char* source, uint32_t nowMs);

  /**
   * Shows the state of the session that owns the matrix, or OFF when no session is left. While
   * LOADING is shown it re-picks the loading picture instead, so a display decision that replaces
   * the comet lands without leaving LOADING.
   * @param {const char*} session - session that changed, nullptr when nothing was requested
   * @param {const char*} source - what triggered the change, logged in brackets
   * @returns {void}
   */
  void refresh(const char* session, const char* source);

  /**
   * Switches the shown animation, a repeated state only restarts when its session asked for it and
   * a change between split and full canvas picture always switches. A running layout slide or
   * strip wipe lands first. A change between the full clock face and a split picture slides when
   * slidesInto() allows it, every other change starts the picture at once: one that is not split
   * on a dark canvas, a split one beside the side clock, which starts unless it already runs. A
   * change to or from the battery face wipes it in when wipesInto() allows it. A change to IDLE
   * first lets returnToClockOnIdle() move the view.
   * @param {PunktState} state - state to show
   * @param {uint8_t} activity - Activity value the session reported
   * @param {int8_t} animation - WorkingAnimation override, -1 when the activity mapping applies
   * @param {const char*} session - session that asked, nullptr keeps a running animation
   * @param {const char*} source - what triggered the change, logged in brackets
   * @returns {void}
   */
  void show(PunktState state, uint8_t activity, int8_t animation, const char* session,
            const char* source);

  /**
   * Starts animation_ the way an instant switch does, a picture that is not split on a dark canvas,
   * a split one beside the side clock.
   * @param {bool} sideClockRuns - true when the side clock already runs and keeps its half
   * @returns {void}
   */
  void startPicture(bool sideClockRuns);

  /**
   * Returns whether a switch to the next picture slides: a change between the full clock face and
   * a split picture on a board with a clock half, while no reveal holds the display.
   * @param {const Picture&} next - picture the switch leads to, compared with the one shown
   * @returns {bool} - true when the layout change may slide, the caller still needs a synced time
   */
  bool slidesInto(const Picture& next) const;

  /**
   * Returns whether a switch to the next picture wipes along the strip: a change between the
   * battery face and the full clock face or a split picture, while no reveal holds the display.
   * @param {const Picture&} next - picture the switch leads to, compared with the one shown
   * @returns {bool} - false on a board without the battery view
   */
  bool wipesInto(const Picture& next) const;

  /**
   * Starts the layout slide into the picture show() just switched to, held at the given time.
   * @param {const tm&} local - local time the slide draws the clock with
   * @returns {void}
   */
  void startSlide(const tm& local);

  /**
   * Lands a running layout slide at once in the state an instant switch would have produced.
   * @returns {void}
   */
  void completeSlide();

  /**
   * Drives a running layout slide and starts animation_ when the slide asks for it.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {bool} - true while the slide holds the frame, so neither animation may tick
   */
  bool advanceSlide(uint32_t nowMs);

  /**
   * Returns whether a running layout slide holds animation_ back, which has then not begun and
   * still reports the finished() of its last run.
   * @returns {bool} - true until the slide starts the picture
   */
  bool pictureHeld() const;

  /**
   * Resets the anonymous session to IDLE with a fresh timestamp, so the matrix rests on the clock
   * once the last named session signed off or the watchdog dropped it.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void restAnonymous(uint32_t nowMs);

  /**
   * Ends a DONE once its animation reported finished(), or right away while the view shows another
   * animation in its place: the session falls back to IDLE so the clock shows.
   * @returns {void}
   */
  void settleDone();

  /**
   * Pushes the context window fill, or 0 while it is stale, into every animation once it
   * changed, so the gauge hides itself without a redraw being forced.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void updateContextPush(uint32_t nowMs);

  /**
   * Pushes the usage share, or 0 while it is stale, into the usage view.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void updateUsagePush(uint32_t nowMs);

  /**
   * Ends the no-session glitch flash once its time is up and puts the view's picture back.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void expireNoSessionFlash(uint32_t nowMs);

  /**
   * Switches the view without counting as activity, the path the board's own view changes take.
   * Leaving OFF clears the off reason, entering it remembers the view to come back to.
   * @param {View} view - view to enter
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {bool} - true when the view changed, false for a repeated or a refused view
   */
  bool changeView(View view, uint32_t nowMs);

  /**
   * Falls back to the CLOCK view once clockTimeoutMin passed without activity. Only NORMAL and
   * USAGE fall back, and only while a named session is registered: without one the matrix
   * already shows the clock.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void applyClockTimeout(uint32_t nowMs);

  /**
   * Returns from the BATTERY view to CLOCK once kBatteryViewMs passed without activity, without
   * counting as activity, so the strip wipe plays unless a touch gesture holds the display.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void applyBatteryTimeout(uint32_t nowMs);

  /**
   * Falls back from NORMAL to the CLOCK view once the shown state turns IDLE from any other state,
   * without counting as activity like the clock timeout, so on a board with a clock half the
   * layout slide plays unless a touch gesture holds the display. A NORMAL view entered while the
   * state already was IDLE stays, USAGE and OFF are never touched. A switch to UNREAD from a busy
   * state falls back the same way, from IDLE or UNREAD it stays.
   * @param {PunktState} state - state show() is about to switch to, compared with the one shown
   * @returns {void}
   */
  void returnToClockOnIdle(PunktState state);

  /**
   * Turns the low-battery dot of the full clock face and the split clock on below 15 percent on
   * battery and off again at 18 percent or on USB, read from VbusSense on every tick. Does nothing
   * on a board without VBUS sense.
   * @returns {void}
   */
  void applyLowBattery();

  /**
   * Lets the clock face about to show draw the unread marker pixel, every other clock hides it.
   * @param {const Animation&} shown - animation show() is about to switch to
   * @param {PunktState} state - state of the displayed session
   * @returns {void}
   */
  void applyClockMarker(const Animation& shown, PunktState state);

  /**
   * Darkens the matrix once the clock rested for the dim timeout plus the off timeout, the last
   * stage of the display cascade. A zero off timeout never darkens it.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void applyOffTimeout(uint32_t nowMs);

  /**
   * Ends a preview at kPreviewMs, or kPreviewHoldMs after a one-shot reported finished(), and
   * puts the picture from before back. Drives the progress sweep while it runs.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void expirePreview(uint32_t nowMs);

  /**
   * Moves a previewed reset fill or OTA spiral from 0 to kPreviewPixels over kPreviewMs, the two
   * animations that show nothing without a progress value. A running reset or OTA owns its own
   * progress and is left alone.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void sweepPreview(uint32_t nowMs);

  /**
   * Returns the animation the current view shows for a state, the gauge replaces every state
   * that does not break through it. A running preview replaces the view itself, only the reset
   * fill, the link pictures and the OTA spiral rank above it. On a board with a clock half the
   * session pictures are split, the system pictures, loading, off, the clock face and the battery
   * face own the whole canvas.
   * @param {PunktState} state - state to look up
   * @param {uint8_t} activity - Activity value the session reported
   * @param {int8_t} animation - WorkingAnimation override, -1 when the activity mapping applies
   * @returns {Picture} - animation owned by this machine, split never set on punkt
   */
  Picture displayAnimationFor(PunktState state, uint8_t activity, int8_t animation);

  /**
   * Returns whether an animation draws the whole canvas, so a preview of it must not be split.
   * @param {const Animation&} animation - animation owned by this machine
   * @returns {bool} - true for the reset fill, the OTA spiral, the Wi-Fi pictures and off
   */
  bool ownsCanvas(const Animation& animation) const;

  /**
   * Returns the clock face the CLOCK view and an empty board rest on, the full canvas face on a
   * board with a clock half.
   * @returns {Animation&} - animation owned by this machine
   */
  Animation& clockFace();

  /**
   * Starts the side clock in the left half, the viewport is moved there for the first frame and
   * back to Display::kViewportX after it.
   * @returns {void}
   */
  void beginSideClock();

  /**
   * Advances the side clock in the left half, the viewport is moved there for the step and back
   * to Display::kViewportX after it.
   * @param {uint32_t} nowMs - millis() at the time of the call
   * @returns {void}
   */
  void tickSideClock(uint32_t nowMs);

  /**
   * Returns the animation a state maps to.
   * @param {PunktState} state - state to look up
   * @param {uint8_t} activity - Activity value the session reported
   * @param {int8_t} animation - WorkingAnimation override, -1 when the activity mapping applies
   * @returns {Animation&} - animation owned by this machine
   */
  Animation& animationFor(PunktState state, uint8_t activity, int8_t animation);

  /**
   * Returns the animation the WORKING state is shown with, the override first and the animation
   * the settings map the activity to otherwise.
   * @param {uint8_t} activity - Activity value the session reported
   * @param {int8_t} animation - WorkingAnimation override, -1 when the activity mapping applies
   * @returns {Animation&} - animation owned by this machine
   */
  Animation& workingAnimation(uint8_t activity, int8_t animation);

  Display& display_;
  Settings& settings_;
  SessionTable sessions_;
  Off off_;
  Orbit orbit_;
  /** comet around the whole canvas border while loading, not part of the preview list */
  Orbit loadingOrbit_{true};
  Clock clock_;
  Tetris tetris_;
  Life life_;
  Knight knight_;
  CodeLines codeLines_;
  MatrixRain matrixRain_;
  Scan scan_;
  Pipes pipes_;
  Pointer pointer_;
  Pong pong_;
  Terminal terminal_;
  Attention attention_;
  Done done_;
  Error error_;
  Waiting waiting_;
  Compacting compacting_;
  Limit limit_;
  Unread unread_;
  Glitch glitch_;
  UsageView usageView_;
  ResetFill resetFill_;
  OtaSpiral otaSpiral_;
  WifiSearch wifiSearch_;
  WifiLost wifiLost_;
  /** split clock in the left half beside a split picture, not part of the preview list */
  SideClock sideClock_;
  /** full canvas clock face of a board with a clock half, not part of the preview list */
  WideClock wideClock_;
  /** idle picture of a board with a clock half, not part of the preview list */
  IdleGauge idleGauge_;
  /** moves the clock between the full face and the left half of a split picture */
  LayoutSlide slide_;
  /** charge picture of the BATTERY view, not part of the preview list */
  BatteryFace batteryFace_;
  /** wipes the battery face in or out beside the clock face and the split pictures */
  CanvasWipe stripWipe_;
  const std::array<Animation*, kAnimationCount> all_ =
      animationList(&off_, &orbit_, &clock_, &tetris_, &life_, &knight_, &codeLines_,
                    &matrixRain_, &scan_, &pipes_, &pointer_, &pong_, &terminal_, &attention_,
                    &done_, &error_, &waiting_, &compacting_, &limit_, &unread_, &glitch_,
                    &usageView_, &resetFill_, &otaSpiral_, &wifiSearch_, &wifiLost_);
  Animation* animation_ = &off_;
  /** the side clock runs in the left half while animation_ runs in the viewport */
  bool split_ = false;
  /** brightness ceiling in effect, the day level or the level the automatic dimming computed */
  uint8_t ceiling_ = Settings::kDefaultBrightness;
  PunktState state_ = PunktState::OFF;
  View view_ = View::CLOCK;
  /** view OFF was entered from, the one toggleOff() lights the matrix again with */
  View viewBeforeOff_ = View::CLOCK;
  /** why the matrix is dark, NONE while the view is not OFF */
  OffReason offReason_ = OffReason::NONE;
  /** the dimmed ceiling is in effect, a flag the status body reports */
  bool dimmed_ = false;
  /** the battery runs low, the clock faces blink their low-battery dot */
  bool lowBattery_ = false;
  /** the BOOT button is held towards the factory reset, the fill overrides everything */
  bool resetting_ = false;
  /** an OTA update is running, its fill overrides everything but the reset and the link pictures */
  bool updating_ = false;
  /** what the station link is doing, PORTAL and LOST replace the picture the view would show */
  WifiSetup::LinkState link_ = WifiSetup::LinkState::CONNECTING;
  /** millis() until which the glitch overrides the view, 0 when no flash runs */
  uint32_t noSessionFlashUntilMs_ = 0;
  /** animation a preview shows over the view, nullptr while no preview runs */
  Animation* preview_ = nullptr;
  /** millis() the running preview started at, the progress sweep counts from it */
  uint32_t previewStartedMs_ = 0;
  /** millis() the running preview ends at, 0 while no preview runs */
  uint32_t previewUntilMs_ = 0;
  /** millis() the previewed animation first reported finished(), 0 while it still plays */
  uint32_t previewFinishedMs_ = 0;
  /** millis() of the last API request, touch, button press, view change or preview */
  uint32_t lastActivityMs_ = 0;
  ReportedPercent context_;
  ReportedPercent usage_;
  uint8_t pushedContext_ = 0;
  bool loading_ = false;
  uint32_t enteredMs_ = 0;
};
