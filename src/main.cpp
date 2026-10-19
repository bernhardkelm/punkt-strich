#include "Button.h"
#include "CoinInput.h"
#include "Commands.h"
#include "Display.h"
#include "ResetHold.h"
#include "SerialCommands.h"
#include "StatusLed.h"
#include "TapMemory.h"
#include "app/CommandDrain.h"
#include "app/SettingsApplier.h"
#include "app/TouchDispatch.h"
#include "app/otaProgress.h"
#include "board/boardConfig.h"
#include "dimming/AutoDim.h"
#include "dimming/BrightnessCeiling.h"
#include "dimming/RestDim.h"
#include "inputActions.h"
#include "network/BootCounter.h"
#include "network/Ota.h"
#include "network/TimeSync.h"
#include "network/WifiSetup.h"
#include "overlay/TouchOverlay.h"
#include "power/BatterySleep.h"
#include "power/NightOff.h"
#include "power/NightSleep.h"
#include "power/Power.h"
#include "power/VbusSense.h"
#include "power/deepSleep.h"
#include "settings/Settings.h"
#include "state/State.h"
#include "touch/FingerTracker.h"
#include "touch/Touch.h"
#include "version.h"
#include "web/HttpServer.h"

#include <Arduino.h>

namespace {

constexpr uint32_t kRetryMs = 2000;
constexpr uint32_t kSaveFlashMs = 300;

Display display;
TouchOverlay touchOverlay(display);
Settings settings;
StateMachine state(display, settings);
Commands commands;
Button button;
ResetHold resetHold(state, settings);
/** punkt's only coin, the left coin on strich */
Touch touch(Touch::kLeftPin, Board::kHasClockHalf ? "touch left" : "touch");
Touch rightTouch(Board::kRightCoinPin, "touch right");
/** gesture source of strich's two coins, never begun on punkt */
FingerTracker fingers(Touch::kLeftPin, Board::kRightCoinPin);
TapMemory tapMemory;
CoinInput coinInput(fingers, state, touchOverlay, tapMemory);
AutoDim autoDim;
RestDim restDim;
BrightnessCeiling ceiling(settings, state, autoDim, restDim);
SettingsApplier settingsApplier(settings, state, ceiling, touch, rightTouch, fingers, touchOverlay);
TouchDispatch touchDispatch(touch, rightTouch, fingers, coinInput, touchOverlay, tapMemory, state);
CommandDrain commandDrain(commands, settings, state, settingsApplier);

/**
 * Lights the whole canvas for a moment so a portal save is acknowledged, blocking is fine because
 * the board restarts right after.
 * @returns {void}
 */
void flashSaved() {
  display.setBrightnessNow(settings.brightness());
  display.clearCanvas();

  for (int8_t y = 0; y < static_cast<int8_t>(Display::kHeight); y++) {
    for (int8_t x = 0; x < static_cast<int8_t>(Display::kCanvasWidth); x++) {
      display.setCanvasPixel(x, y, true);
    }
  }

  display.tick(millis());
  display.show();
  delay(kSaveFlashMs);
}

}

void setup() {
  Serial.begin(115200);
  logSleepWake();
  // ahead of the LED and the matrix probe, a USB check wake on battery sleeps again with both dark
  BatterySleep::recheckUsb();
  // the WS2812 holds its last colour through a restart, clear it
  StatusLed::off();

  // the factory reset runs before the matrix probe, the retry loop never returns without hardware
  if (BootCounter::begin() >= BootCounter::kResetCount) {
    Serial.println("punkt: factory reset, erasing wifi credentials");
    BootCounter::reset();
    WifiSetup::forget();
    ESP.restart();
  }

  while (!display.begin(Board::kSdaPin, Board::kSclPin)) {
    Serial.printf("punkt: matrix not found at 0x%02X on SDA=%u SCL=%u, retrying\n",
                  display.failedAddress(), Board::kSdaPin, Board::kSclPin);
    delay(kRetryMs);
  }

  Serial.printf("punkt: matrix ok, version %s\n", kFirmwareVersion);
  settings.begin();
  Serial.printf("punkt: name %s\n", settings.name());
  // the RTC keeps the time through a deep sleep but not the zone, SNTP sets it only with Wi-Fi
  settingsApplier.beginTimezone();
  // the CPU clock and the stored sleep mode must be set before the Wi-Fi driver starts
  settingsApplier.beginPowerSave();
  button.begin();
  VbusSense::begin();
  touch.begin(millis());
  settingsApplier.applyTouch(touch);
  touchOverlay.setEnabled(settings.touchHover() != 0);

  // nothing was drawn yet, an unconfirmed wake returns to sleep with the matrix still dark
  if (NightSleep::wokeByTouch()) {
    NightSleep::confirmWake(touch, display, settings);
    // the wake is the whole gesture, a DOUBLE_TAP that still follows must not darken the matrix
    tapMemory.effect = TapEffect::WOKE;
  }

  // begun after the wake confirmation, their calibration would stall the left coin's polling
  if (Board::kHasClockHalf) {
    rightTouch.begin(millis());
    settingsApplier.applyTouch(rightTouch);
    fingers.begin(millis());
    settingsApplier.applyFingers();
  }

  state.begin(millis());
  ceiling.refresh();
  WifiSetup::begin(settings.name());
}

void loop() {
  const uint32_t nowMs = millis();

  if (WifiSetup::process(nowMs, flashSaved)) {
    beginOta(settings.name(), state);
    // the WiFiManager portal owns port 80 while the AP runs, it is stopped on the first connection
    HttpServer::begin(commands, settings, state, touch, rightTouch);
    TimeSync::begin(settings.timezone());
    state.set(PunktState::IDLE, "wifi", nowMs);
  }

  state.setLink(WifiSetup::link(nowMs), nowMs);
  Ota::handle();
  HttpServer::tick(nowMs);
  BootCounter::tick(nowMs);
  pollSerialCommands(commands, settings);
  commandDrain.drain(nowMs);

  // a release after a long hold ends the reset fill and is not a click
  if (button.tick(nowMs)) {
    state.noteActivity(nowMs);

    if (button.lastHoldMs() < ResetHold::kClickMaxMs) {
      handleButtonRelease(state, settings, commands, nowMs);
    }
  }

  resetHold.tick(max(button.heldMs(nowMs), touchDispatch.resetHeldMs(nowMs)), nowMs);
  touchDispatch.tick(nowMs);
  Power::tick(nowMs);
  VbusSense::tick(nowMs);

  if (Board::kHasClockHalf && VbusSense::consumeUsbChange()) {
    Serial.println("punkt: usb power changed, rebasing the coins");
    fingers.powerChanged(nowMs);
  }

  TimeSync::tick(nowMs);
  ceiling.tick(nowMs);
  touchDispatch.tickOverlay(ceiling.level(), nowMs);
  state.tick(nowMs);

  NightOff::tick(nowMs, settings, state);

  if (NightSleep::due(nowMs, settings, state)) {
    NightSleep::enter(display, touch, settings);
  }

  if (BatterySleep::due(nowMs, settings, state)) {
    BatterySleep::enter(display, touch);
  }

  // one tick of sleep hands the core to the idle task, every module above is time based
  delay(1);
}
