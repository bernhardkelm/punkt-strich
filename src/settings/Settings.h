#pragma once

#include "board/boardConfig.h"

#include <Preferences.h>
#include <array>
#include <cstddef>
#include <cstdint>

/** Animation shown while the WORKING state is active. */
enum class WorkingAnimation : uint8_t {
  TETRIS,
  LIFE,
  KNIGHT,
  ORBIT,
  CODE,
  RAIN,
  SCAN,
  PIPES,
  POINTER,
  PONG,
  TERMINAL
};

/** Number of WorkingAnimation enumerators, the button cycles through them. */
constexpr uint8_t kWorkingAnimationCount = 11;

/** Full-width clock face shown on a board with a clock half. */
enum class ClockFace : uint8_t { STAIRCASE, TALL };

/** Number of ClockFace enumerators. */
constexpr uint8_t kClockFaceCount = 2;

/** Style of the split clock beside the Claude picture on a board with a clock half. */
enum class SplitClock : uint8_t { DIGITS, SQUARES, STAGGER, BAR };

/** Number of SplitClock enumerators. */
constexpr uint8_t kSplitClockCount = 4;

/** What a session reports it is doing, the settings map it to an animation. */
enum class Activity : uint8_t { THINKING, READING, EDITING, SHELL, DELEGATING, MCP };

/** Number of Activity enumerators, the settings hold one animation per activity. */
constexpr uint8_t kActivityCount = 6;

/** Activity of a session that never reported one, and of every request without the field. */
constexpr uint8_t kDefaultActivity = static_cast<uint8_t>(Activity::THINKING);

/** Animation an activity falls back to, indexed by the Activity value. */
constexpr std::array<WorkingAnimation, kActivityCount> kDefaultActivityAnimations = {
    WorkingAnimation::KNIGHT,   WorkingAnimation::SCAN,  WorkingAnimation::CODE,
    WorkingAnimation::TERMINAL, WorkingAnimation::ORBIT, WorkingAnimation::POINTER};

/** NVS backed settings, cached as PODs because loop() writes them while the HTTP task reads. */
class Settings {
public:
  static constexpr uint8_t kMaxBrightness = 15;
  static constexpr uint16_t kMaxTimeoutMin = 1440;
  static constexpr uint8_t kDefaultBrightness = Board::kDefaultBrightness;
  static constexpr uint16_t kDefaultOffTimeoutMin = 0;
  static constexpr uint16_t kMaxBatterySleepMin = 240;
  static constexpr uint16_t kDefaultBatterySleepMin = 15;
  static constexpr SplitClock kDefaultSplitClock = SplitClock::DIGITS;
  static constexpr uint16_t kDefaultClockTimeoutMin = 5;
  static constexpr uint16_t kDefaultDimTimeoutMin = Board::kDefaultDimTimeoutMin;
  static constexpr bool kDefaultPowerSave = true;
  static constexpr bool kDefaultDebugTouch = false;
  static constexpr bool kDefaultNightSleep = false;
  static constexpr uint16_t kMaxClockMin = 1439;
  static constexpr uint16_t kDefaultSleepAt = 1320;
  static constexpr bool kDefaultNightOff = false;
  static constexpr uint16_t kDefaultWakeAt = 420;
  static constexpr bool kDefaultAutoDim = Board::kDefaultAutoDim;
  static constexpr ClockFace kDefaultClockFace =
      static_cast<ClockFace>(Board::kDefaultClockFace);
  static constexpr uint8_t kDefaultNightBrightness = Board::kDefaultNightBrightness;
  static constexpr uint8_t kMinTouchTrigger = 5;
  static constexpr uint8_t kMaxTouchTrigger = 250;
  static constexpr uint8_t kDefaultTouchTrigger = Board::kDefaultTouchTrigger;
  static constexpr uint8_t kMaxTouchHover = 50;
  static constexpr uint8_t kDefaultTouchHover = 8;
  /** Longest POSIX TZ rule the table carries, callers size their copy buffers with it. */
  static constexpr size_t kTimezoneMax = 48;
  static constexpr size_t kCityNameMax = 32;
  static constexpr size_t kNameMax = 24;
  static constexpr const char* kDefaultName = Board::kDefaultName;

  /** Opens the NVS namespace, loads every value and writes the defaults for missing keys. */
  void begin();

  /** Erases every stored setting, the caller restarts so begin() writes the defaults again. */
  void reset();

  /**
   * Returns the brightness ceiling animations scale against.
   * @returns {uint8_t} - 0 to 15
   */
  uint8_t brightness() const;

  /**
   * Stores the brightness ceiling, clamped and persisted immediately.
   * @param {uint8_t} level - 0 to 15, higher values are clamped
   * @returns {void}
   */
  void setBrightness(uint8_t level);

  /**
   * Returns the minutes the clock face rests before the brightness ceiling drops to the night
   * brightness.
   * @returns {uint16_t} - minutes, 0 disables the timeout
   */
  uint16_t dimTimeoutMin() const;

  /**
   * Stores the dim timeout, clamped and persisted immediately.
   * @param {uint16_t} minutes - 0 to 1440, 0 disables the timeout
   * @returns {void}
   */
  void setDimTimeoutMin(uint16_t minutes);

  /**
   * Returns the minutes the resting clock stays on after the dim stage before the screen switches
   * off.
   * @returns {uint16_t} - minutes, 0 disables the timeout
   */
  uint16_t offTimeoutMin() const;

  /**
   * Stores the off timeout, clamped and persisted immediately.
   * @param {uint16_t} minutes - 0 to 1440, 0 disables the timeout
   * @returns {void}
   */
  void setOffTimeoutMin(uint16_t minutes);

  /**
   * Returns the minutes without activity after which a board running on battery goes to deep
   * sleep.
   * @returns {uint16_t} - minutes, 0 disables the battery sleep
   */
  uint16_t batterySleepMin() const;

  /**
   * Stores the battery sleep timeout, clamped and persisted immediately.
   * @param {uint16_t} minutes - 0 to 240, 0 disables the battery sleep
   * @returns {void}
   */
  void setBatterySleepMin(uint16_t minutes);

  /**
   * Returns the minutes without an API command or a view change after which the view falls back
   * to CLOCK while a named session is registered.
   * @returns {uint16_t} - minutes, 0 disables the timeout
   */
  uint16_t clockTimeoutMin() const;

  /**
   * Stores the clock timeout, clamped and persisted immediately.
   * @param {uint16_t} minutes - 0 to 1440, 0 disables the timeout
   * @returns {void}
   */
  void setClockTimeoutMin(uint16_t minutes);

  /**
   * Returns the animation an activity is shown with while the WORKING state is active.
   * @param {Activity} activity - activity to look up
   * @returns {WorkingAnimation} - TETRIS, LIFE, KNIGHT, ORBIT, CODE, RAIN, SCAN, PIPES, POINTER,
   * PONG or TERMINAL
   */
  WorkingAnimation activityAnimation(Activity activity) const;

  /**
   * Stores the animation of one activity, an unknown animation falls back to that activity's
   * default, an unknown activity is ignored.
   * @param {Activity} activity - activity the animation belongs to
   * @param {WorkingAnimation} animation - animation to show while the activity runs
   * @returns {void}
   */
  void setActivityAnimation(Activity activity, WorkingAnimation animation);

  /**
   * Returns whether the CPU runs at 80 MHz with Wi-Fi modem sleep.
   * @returns {bool} - true when power saving is on
   */
  bool powerSave() const;

  /**
   * Stores the power save flag, persisted immediately.
   * @param {bool} enabled - true for 80 MHz and modem sleep, false for 240 MHz and no sleep
   * @returns {void}
   */
  void setPowerSave(bool enabled);

  /**
   * Returns whether the touch electrode streams its raw value and baseline over serial.
   * @returns {bool} - true when the debug stream is on
   */
  bool debugTouch() const;

  /**
   * Stores the touch debug flag, persisted immediately.
   * @param {bool} enabled - true streams raw and baseline every 200 ms
   * @returns {void}
   */
  void setDebugTouch(bool enabled);

  /**
   * Returns whether the board enters deep sleep inside the night window, only effective while
   * nightOff is on.
   * @returns {bool} - true when the night sleep is on
   */
  bool nightSleep() const;

  /**
   * Stores the night sleep flag, persisted immediately.
   * @param {bool} enabled - true enters deep sleep inside the night window while nightOff is on
   * @returns {void}
   */
  void setNightSleep(bool enabled);

  /**
   * Returns the local time the night sleep window opens.
   * @returns {uint16_t} - minutes since midnight, 0 to 1439
   */
  uint16_t sleepAt() const;

  /**
   * Stores the night sleep time, clamped and persisted immediately.
   * @param {uint16_t} minutes - minutes since midnight, higher values are clamped to 1439
   * @returns {void}
   */
  void setSleepAt(uint16_t minutes);

  /**
   * Returns whether the screen switches off inside the night window from sleepAt to wakeAt.
   * @returns {bool} - true when the night off is on
   */
  bool nightOff() const;

  /**
   * Stores the night off flag, persisted immediately.
   * @param {bool} enabled - true switches the screen off inside the night window
   * @returns {void}
   */
  void setNightOff(bool enabled);

  /**
   * Returns the full-width clock face a board with a clock half shows.
   * @returns {ClockFace} - STAIRCASE or TALL
   */
  ClockFace clockFace() const;

  /**
   * Stores the clock face, an unknown value falls back to STAIRCASE, persisted immediately.
   * @param {ClockFace} face - clock face to show
   * @returns {void}
   */
  void setClockFace(ClockFace face);

  /**
   * Returns the style of the split clock a board with a clock half shows beside the Claude picture.
   * @returns {SplitClock} - DIGITS, SQUARES, STAGGER or BAR
   */
  SplitClock splitClock() const;

  /**
   * Stores the split clock style, an unknown value falls back to DIGITS, persisted immediately.
   * @param {SplitClock} style - split clock style to show
   * @returns {void}
   */
  void setSplitClock(SplitClock style);

  /**
   * Returns the local time the night window ends, always, and the time the deep sleep timer wakes
   * the board.
   * @returns {uint16_t} - minutes since midnight, 0 to 1439
   */
  uint16_t wakeAt() const;

  /**
   * Stores the wake time, clamped and persisted immediately.
   * @param {uint16_t} minutes - minutes since midnight, higher values are clamped to 1439
   * @returns {void}
   */
  void setWakeAt(uint16_t minutes);

  /**
   * Returns whether the brightness ceiling follows the sun's elevation.
   * @returns {bool} - true when the automatic dimming is on
   */
  bool autoDim() const;

  /**
   * Stores the automatic dimming flag, persisted immediately.
   * @param {bool} enabled - true scales the ceiling with the sun's elevation at the stored place
   * @returns {void}
   */
  void setAutoDim(bool enabled);

  /**
   * Returns the brightness ceiling at night while autoDim is on, and the ceiling of a resting
   * clock.
   * @returns {uint8_t} - 0 to 15
   */
  uint8_t nightBrightness() const;

  /**
   * Stores the night brightness ceiling, clamped and persisted immediately.
   * @param {uint8_t} level - 0 to 15, higher values are clamped
   * @returns {void}
   */
  void setNightBrightness(uint8_t level);

  /**
   * Returns the latitude of the city the board sits in.
   * @returns {float} - degrees, north positive, -90 to 90
   */
  float latitude() const;

  /**
   * Returns the longitude of the city the board sits in.
   * @returns {float} - degrees, east positive, -180 to 180
   */
  float longitude() const;

  /**
   * Returns how far above its baseline the touch reading must rise to count as a touch.
   * @returns {uint8_t} - percent, 5 to 250
   */
  uint8_t touchTrigger() const;

  /**
   * Stores the touch trigger, clamped to 5 to 250 and persisted immediately.
   * @param {uint8_t} percent - percent above the baseline
   * @returns {void}
   */
  void setTouchTrigger(uint8_t percent);

  /**
   * Returns where the hover dome starts, as percent of the touch trigger margin.
   * @returns {uint8_t} - percent, 0 to 50, 0 turns the dome and the ripple off
   */
  uint8_t touchHover() const;

  /**
   * Stores the touch hover start, clamped to 0 to 50 and persisted immediately.
   * @param {uint8_t} percent - percent of the touch trigger margin, 0 turns the dome and the
   * ripple off
   * @returns {void}
   */
  void setTouchHover(uint8_t percent);

  /**
   * Returns the IANA name of the city the board sits in, timezone and coordinates derive from it.
   * @returns {const char*} - at most 32 characters, always a name the city table knows
   */
  const char* city() const;

  /**
   * Stores the city, persisted immediately, an empty or unknown name restores the default.
   * @param {const char*} name - IANA name such as "Europe/Berlin", may be null
   * @returns {void}
   */
  void setCity(const char* name);

  /**
   * Returns the POSIX TZ rule of the city, the one the clock runs on.
   * @returns {const char*} - at most kTimezoneMax characters, never empty
   */
  const char* timezone() const;

  /**
   * Returns the board name, used as the hostname, the mDNS host and the setup network name.
   * @returns {const char*} - at most 24 characters, never empty
   */
  const char* name() const;

  /**
   * Stores the board name, truncated to 24 characters, an empty string restores the default. The
   * network side reads it at boot, so a change reaches the hostname on the next restart.
   * @param {const char*} value - lowercase letters, digits and hyphens, such as "punkt-work"
   * @returns {void}
   */
  void setName(const char* value);

private:
  /** Size of an activity animation key, "anim" plus one digit and the terminator. */
  static constexpr size_t kAnimationKeySize = 6;
  static constexpr const char* kNamespace = "punkt";
  static constexpr const char* kBrightnessKey = "bright";
  static constexpr const char* kDimTimeoutKey = "dimMin";
  static constexpr const char* kOffTimeoutKey = "offMin";
  static constexpr const char* kClockTimeoutKey = "clockMin";
  static constexpr const char* kPowerSaveKey = "pwrSave";
  static constexpr const char* kDebugTouchKey = "dbgTouch";
  static constexpr const char* kNightSleepKey = "nightSlp";
  static constexpr const char* kSleepAtKey = "sleepAt";
  static constexpr const char* kNightOffKey = "nightOff";
  static constexpr const char* kWakeAtKey = "wakeAt";
  static constexpr const char* kAutoDimKey = "autoDim";
  static constexpr const char* kNightBrightnessKey = "nightBr";
  static constexpr const char* kTouchTriggerKey = "touchTrig";
  static constexpr const char* kTouchHoverKey = "touchHover";
  static constexpr const char* kClockFaceKey = "clockFace";
  static constexpr const char* kSplitClockKey = "splitClock";
  static constexpr const char* kBatterySleepKey = "battSleepMin";
  static constexpr const char* kCityKey = "city";
  static constexpr const char* kNameKey = "name";

  /**
   * Writes the NVS key an activity animation is stored under.
   * @param {Activity} activity - activity the key belongs to
   * @param {char*} out - buffer of at least kAnimationKeySize characters
   * @returns {void}
   */
  static void animationKey(Activity activity, char* out);

  /**
   * Caches the animation of one activity without writing NVS, an unknown animation falls back to
   * that activity's default.
   * @param {Activity} activity - activity the animation belongs to
   * @param {WorkingAnimation} animation - animation to show while the activity runs
   * @returns {bool} - false when the activity is out of range and nothing was cached
   */
  bool cacheActivityAnimation(Activity activity, WorkingAnimation animation);

  /**
   * Loads every activity animation, an activity without a stored key keeps its default and stays
   * unwritten so a later default change still reaches the board.
   * @returns {void}
   */
  void loadActivityAnimations();

  Preferences prefs_;
  uint8_t brightness_ = kDefaultBrightness;
  uint16_t dimTimeoutMin_ = kDefaultDimTimeoutMin;
  uint16_t offTimeoutMin_ = kDefaultOffTimeoutMin;
  uint16_t batterySleepMin_ = kDefaultBatterySleepMin;
  uint16_t clockTimeoutMin_ = kDefaultClockTimeoutMin;
  std::array<WorkingAnimation, kActivityCount> activityAnimation_ = kDefaultActivityAnimations;
  bool powerSave_ = kDefaultPowerSave;
  bool debugTouch_ = kDefaultDebugTouch;
  bool nightSleep_ = kDefaultNightSleep;
  uint16_t sleepAt_ = kDefaultSleepAt;
  bool nightOff_ = kDefaultNightOff;
  ClockFace clockFace_ = kDefaultClockFace;
  SplitClock splitClock_ = kDefaultSplitClock;
  uint16_t wakeAt_ = kDefaultWakeAt;
  bool autoDim_ = kDefaultAutoDim;
  uint8_t nightBrightness_ = kDefaultNightBrightness;
  uint8_t touchTrigger_ = kDefaultTouchTrigger;
  uint8_t touchHover_ = kDefaultTouchHover;
  char city_[kCityNameMax + 1] = {};
  char name_[kNameMax + 1] = {};
};
