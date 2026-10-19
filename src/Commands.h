#pragma once

#include "settings/Settings.h"
#include "state/State.h"

#include <atomic>
#include <cstdint>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

/** One queued state request, a POD because the FreeRTOS queue copies it by value. */
struct StateRequest {
  /** Session id, empty for the serial console, the settings page and the Makefile. */
  char session[kSessionIdMax + 1];
  PunktState state;
  /** Activity value the request reported, kDefaultActivity when it carried none. */
  uint8_t activity;
  /** WorkingAnimation value overriding the activity mapping, -1 when the mapping applies. */
  int8_t animation;
  /** Sender clock in ms when the request was made, 0 when it carries none. */
  uint64_t ts;
};

/**
 * One queued settings patch, a POD because the FreeRTOS queue copies it by value. Every field is
 * already validated and clamped by the sender, loop() hands it to the setters unchecked.
 */
struct SettingsPatch {
  /** Value of every numeric field that the request did not carry. */
  static constexpr int16_t kAbsent = -1;

  /** Brightness ceiling, kAbsent when the field was not sent. */
  int16_t brightness = kAbsent;
  /** Minutes the clock rests before the ceiling drops, kAbsent when the field was not sent. */
  int16_t dimTimeoutMin = kAbsent;
  /** Minutes before a session is dropped, kAbsent when the field was not sent. */
  int16_t offTimeoutMin = kAbsent;
  /** Minutes without activity before a battery powered board sleeps, kAbsent when not sent. */
  int16_t batterySleepMin = kAbsent;
  /** Minutes before the view falls back to the clock, kAbsent when the field was not sent. */
  int16_t clockTimeoutMin = kAbsent;
  /** Percent above the touch baseline, kAbsent when the field was not sent. */
  int16_t touchTrigger = kAbsent;
  /** Percent of the trigger margin the hover dome starts at, kAbsent when the field was not sent. */
  int16_t touchHover = kAbsent;
  /** One WorkingAnimation value per Activity, kAbsent for every activity not sent. */
  int8_t animations[kActivityCount] = {kAbsent, kAbsent, kAbsent, kAbsent, kAbsent, kAbsent};
  /** 1 or 0, kAbsent when the field was not sent. */
  int8_t powerSave = kAbsent;
  /** 1 or 0, kAbsent when the field was not sent. */
  int8_t debugTouch = kAbsent;
  /** 1 or 0, kAbsent when the field was not sent. */
  int8_t nightSleep = kAbsent;
  /** Minutes since midnight, kAbsent when the field was not sent. */
  int16_t sleepAt = kAbsent;
  /** 1 or 0, kAbsent when the field was not sent. */
  int8_t nightOff = kAbsent;
  /** ClockFace value, kAbsent when the field was not sent. */
  int8_t clockFace = kAbsent;
  /** SplitClock value, kAbsent when the field was not sent. */
  int8_t splitClock = kAbsent;
  /** Minutes since midnight, kAbsent when the field was not sent. */
  int16_t wakeAt = kAbsent;
  /** 1 or 0, kAbsent when the field was not sent. */
  int8_t autoDim = kAbsent;
  /** Night brightness ceiling, kAbsent when the field was not sent. */
  int16_t nightBrightness = kAbsent;
  /** City name, empty when the field was not sent, the sender refuses an unknown one. */
  char city[Settings::kCityNameMax + 1] = {};
  /** Board name, empty when the field was not sent, an empty name is refused by the sender. */
  char name[Settings::kNameMax + 1] = {};
};

/** Value of Commands::viewRequest that asks for the next view instead of a specific one. */
constexpr int8_t kViewToggle = 100;

/**
 * Commands handed from the HTTP task to loop(), which drains them once per iteration. Everything
 * that touches NVS or a driver belongs on the loop() side: the HTTP task only validates a request
 * and queues it, so no handler blocks the AsyncTCP task on a flash commit.
 */
struct Commands {
  static constexpr uint8_t kStateQueueLength = 8;
  static constexpr uint8_t kSettingsQueueLength = 4;

  /** Creates the queues, the scheduler already runs when the globals are constructed. */
  Commands();

  /**
   * Queues a state request, a full queue drops it.
   * @param {const char*} session - session id, empty string for the anonymous session
   * @param {PunktState} state - state the session asks for
   * @param {uint8_t} activity - Activity value, kDefaultActivity when the request carries none
   * @param {int8_t} animation - WorkingAnimation override, -1 when the activity mapping applies
   * @param {uint64_t} ts - sender clock in ms, 0 when the request carries none
   * @returns {bool} - true when the request was queued
   */
  bool postState(const char* session, PunktState state, uint8_t activity, int8_t animation,
                 uint64_t ts);

  /**
   * Takes the oldest queued state request.
   * @param {StateRequest&} out - receives the request, untouched when the queue is empty
   * @returns {bool} - true when a request was handed over
   */
  bool takeState(StateRequest& out);

  /**
   * Queues a settings patch, a full queue drops it.
   * @param {const SettingsPatch&} patch - validated fields, absent ones marked as such
   * @returns {bool} - true when the patch was queued
   */
  bool postSettings(const SettingsPatch& patch);

  /**
   * Takes the oldest queued settings patch, loop() applies it through the setters and persists it.
   * @param {SettingsPatch&} out - receives the patch, untouched when the queue is empty
   * @returns {bool} - true when a patch was handed over
   */
  bool takeSettings(SettingsPatch& out);

  /** Context window fill reported through the API, -1 when nothing is pending. */
  std::atomic<int16_t> contextPercent{-1};
  /** Share of the 5 hour usage limit reported through the API, -1 when nothing is pending. */
  std::atomic<int16_t> usagePercent{-1};
  /** Requested display view, -1 when nothing is pending, a View value or kViewToggle. */
  std::atomic<int8_t> viewRequest{-1};
  /** Index of the animation to preview, -1 when nothing is pending. */
  std::atomic<int8_t> previewRequest{-1};
  /** Settings were written and the modules reading them have to pick them up, set on loop(). */
  std::atomic<bool> settingsChanged{false};
  /** The device has to restart, raised by the HTTP task once the response left the board. */
  std::atomic<bool> reboot{false};
  /** Stored Wi-Fi credentials have to be erased, followed by a restart. */
  std::atomic<bool> forgetWifi{false};

private:
  QueueHandle_t stateQueue_;
  QueueHandle_t settingsQueue_;
};
