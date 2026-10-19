#include "network/BootCounter.h"

#include "nameTable.h"

#include <Arduino.h>
#include <Preferences.h>
#include <cstddef>
#include <esp_rom_sys.h>
#include <esp_system.h>

namespace {

constexpr const char* kNamespace = "punkt";
constexpr const char* kKey = "boots";
constexpr uint32_t kWindowMs = 10000;
// index matches the esp_reset_reason_t value
constexpr const char* kResetNames[] = {"unknown", "poweron", "external", "software",
                                       "panic", "int-wdt", "task-wdt", "wdt",
                                       "deepsleep", "brownout", "sdio"};
constexpr size_t kResetNameCount = nameCount(kResetNames);

bool cleared = false;

/**
 * Returns whether a reset counts towards the factory reset, only a power-on or the reset pin
 * can come from the user cycling the power.
 * @param {esp_reset_reason_t} reason - reason of the last reset
 * @returns {bool} - true for ESP_RST_POWERON and ESP_RST_EXT
 */
bool counted(esp_reset_reason_t reason) {
  return reason == ESP_RST_POWERON || reason == ESP_RST_EXT;
}

/**
 * Returns whether the USB serial or JTAG unit reset the chip, IDF 4.4 reports that reset as
 * ESP_RST_UNKNOWN.
 * @returns {bool} - true when the ROM saw a USB UART or USB JTAG core reset
 */
bool usbReset() {
  const soc_reset_reason_t rom = esp_rom_get_reset_reason(0);
  return rom == RESET_REASON_CORE_USB_UART || rom == RESET_REASON_CORE_USB_JTAG;
}

/**
 * Writes the boot count to NVS.
 * @param {uint8_t} count - value to persist
 * @returns {void}
 */
void store(uint8_t count) {
  Preferences prefs;

  if (!prefs.begin(kNamespace, false)) {
    return;
  }

  prefs.putUChar(kKey, count);
  prefs.end();
}

}

namespace BootCounter {

uint8_t begin() {
  Serial.printf("punkt: reset reason %s\n", resetReason());

  if (!counted(esp_reset_reason())) {
    Serial.println("punkt: boot not counted");
    return 1;
  }

  Preferences prefs;

  if (!prefs.begin(kNamespace, false)) {
    return 1;
  }

  const uint8_t count = prefs.getUChar(kKey, 0) + 1;
  prefs.putUChar(kKey, count);
  prefs.end();
  Serial.printf("punkt: boot %u/%u in window\n", count, kResetCount);
  return count;
}

void tick(uint32_t nowMs) {
  if (cleared || nowMs < kWindowMs) {
    return;
  }

  cleared = true;
  store(0);
}

void reset() {
  cleared = true;
  store(0);
}

const char* resetReason() {
  const esp_reset_reason_t reason = esp_reset_reason();

  if (reason == ESP_RST_UNKNOWN && usbReset()) {
    return "usb";
  }

  return nameAt(kResetNames, kResetNameCount, static_cast<size_t>(reason), "unknown");
}

}
