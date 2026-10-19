#include "network/WifiSetup.h"

#include "power/Power.h"
#include "settings/Settings.h"

#include <ESPmDNS.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <cstdio>
#include <esp_wifi.h>

namespace {

constexpr char kPortalSuffix[] = "-setup";
constexpr const char* kMdnsService = "http";
constexpr const char* kMdnsProtocol = "tcp";
constexpr uint16_t kHttpPort = 80;
constexpr uint32_t kPollIntervalMs = 250;
constexpr uint32_t kReconnectAfterMs = 10000;
constexpr uint32_t kConnectTimeoutMs = 15000;
constexpr uint32_t kBootConnectS = 5;
/** full power reflects on the Super Mini's onboard antenna, this level links farther */
constexpr wifi_power_t kTxPower = WIFI_POWER_8_5dBm;

WiFiManager manager;
char hostname[Settings::kNameMax + 1] = {};
char portalSsid[Settings::kNameMax + sizeof(kPortalSuffix)] = {};
bool connected = false;
bool portalRan = false;
bool credentialsSaved = false;
uint32_t beganMs = 0;
uint32_t lastPollMs = 0;
uint32_t lostSinceMs = 0;
bool lost = false;
bool reconnecting = false;
bool connectTimedOut = false;

/**
 * Re-associates once the link has been down for kReconnectAfterMs and restarts that wait.
 * @param {uint32_t} nowMs - millis() at the time of the call
 * @returns {void}
 */
void retryLink(uint32_t nowMs) {
  if (nowMs - lostSinceMs < kReconnectAfterMs) {
    return;
  }

  lostSinceMs = nowMs;
  reconnecting = true;
  Serial.println("punkt: wifi reconnect");
  WiFi.reconnect();
}

/**
 * Re-associates when the station has been down for a while, setAutoReconnect only reacts to a
 * disconnect event and a link that silently dies never raises one.
 * @param {uint32_t} nowMs - millis() at the time of the call
 * @returns {void}
 */
void watchLink(uint32_t nowMs) {
  if (WiFi.status() == WL_CONNECTED) {
    if (lost) {
      lost = false;
      reconnecting = false;
      Serial.printf("punkt: wifi back %s\n", WiFi.localIP().toString().c_str());
    }

    return;
  }

  if (!lost) {
    lost = true;
    lostSinceMs = nowMs;
    Serial.println("punkt: wifi lost");
    return;
  }

  retryLink(nowMs);
}

/**
 * Logs once when the stored credentials did not connect within the boot timeout and from then on
 * re-associates on the watchdog cadence until the station links, the first attempt follows the
 * timeout directly.
 * @param {uint32_t} nowMs - millis() at the time of the call
 * @returns {void}
 */
void retryBootConnect(uint32_t nowMs) {
  if (!credentialsSaved || nowMs - beganMs < kConnectTimeoutMs) {
    return;
  }

  if (!connectTimedOut) {
    connectTimedOut = true;
    lostSinceMs = nowMs - kReconnectAfterMs;
    Serial.println("punkt: link lost, connect timeout");
  }

  retryLink(nowMs);
}

/**
 * Publishes the device as <name>.local with its web service.
 * @returns {void}
 */
void startMdns() {
  if (!MDNS.begin(hostname)) {
    Serial.println("punkt: mdns start failed");
    return;
  }

  MDNS.addService(kMdnsService, kMdnsProtocol, kHttpPort);
}

/**
 * Sets the station radio for range over throughput: reduced transmit power, since the Super Mini
 * antenna reflects at full power, and 20 MHz channels for their better sensitivity. Runs on the
 * Wi-Fi event task at every station start, a driver restart drops both values.
 * @param {arduino_event_id_t} event - always ARDUINO_EVENT_WIFI_STA_START, unused
 * @returns {void}
 */
void applyRadio(arduino_event_id_t event) {
  static_cast<void>(event);
  WiFi.setTxPower(kTxPower);
  esp_wifi_set_bandwidth(WIFI_IF_STA, WIFI_BW_HT20);
}

/**
 * Reads the transmit power and the station bandwidth back from the driver and logs them.
 * @returns {void}
 */
void logRadio() {
  wifi_bandwidth_t bandwidth = WIFI_BW_HT20;
  esp_wifi_get_bandwidth(WIFI_IF_STA, &bandwidth);

  Serial.printf("punkt: wifi tx=%d/4dBm bw=%s\n", static_cast<int>(WiFi.getTxPower()),
                bandwidth == WIFI_BW_HT20 ? "ht20" : "ht40");
}

/**
 * Reads the driver's active power save mode and station listen interval and logs them, Power::apply
 * only stores the requested mode until the driver is up.
 * @returns {void}
 */
void logPowerSave() {
  wifi_ps_type_t mode = WIFI_PS_NONE;
  esp_wifi_get_ps(&mode);

  wifi_config_t conf = {};
  esp_wifi_get_config(WIFI_IF_STA, &conf);

  Serial.printf("punkt: wifi ps=%s listen=%u\n", Power::sleepLabel(mode), conf.sta.listen_interval);
}

}

namespace WifiSetup {

void begin(const char* name) {
  beganMs = millis();
  snprintf(hostname, sizeof(hostname), "%s", name == nullptr ? Settings::kDefaultName : name);
  snprintf(portalSsid, sizeof(portalSsid), "%s%s", hostname, kPortalSuffix);
  // WiFiManager cycles the driver through WIFI_OFF, the radio values must follow every start
  WiFi.onEvent(applyRadio, ARDUINO_EVENT_WIFI_STA_START);
  WiFi.mode(WIFI_STA);
  // the station config the portal saved only reads back once the driver is up
  credentialsSaved = manager.getWiFiIsSaved();
  WiFi.setAutoReconnect(true);
  manager.setDebugOutput(false);
  manager.setHostname(hostname);
  manager.setConfigPortalBlocking(false);
  manager.setCaptivePortalEnable(true);
  // WiFiManager disables the station when it opens the portal, stored credentials keep retrying
  manager.setEnableConfigPortal(!credentialsSaved);
  manager.setConnectTimeout(kBootConnectS);

  if (manager.autoConnect(portalSsid)) {
    return;
  }

  if (credentialsSaved) {
    Serial.println("punkt: wifi connect pending, retrying");
    return;
  }

  portalRan = true;
  Serial.printf("punkt: wifi portal %s\n", portalSsid);
}

bool process(uint32_t nowMs, SavedCallback onSaved) {
  manager.process();

  if (nowMs - lastPollMs < kPollIntervalMs) {
    return false;
  }

  lastPollMs = nowMs;

  if (connected) {
    watchLink(nowMs);
    return false;
  }

  if (WiFi.status() != WL_CONNECTED) {
    retryBootConnect(nowMs);
    return false;
  }

  // the portal's HTTP connections keep port 80 in TIME_WAIT and AsyncTCP binds without
  // SO_REUSEADDR, the next boot connects with the saved credentials and never opens the portal
  if (portalRan) {
    Serial.println("punkt: wifi saved, restarting");
    Serial.flush();

    if (onSaved != nullptr) {
      onSaved();
    }

    ESP.restart();
  }

  connected = true;
  reconnecting = false;
  startMdns();
  Serial.printf("punkt: wifi connected %s\n", WiFi.localIP().toString().c_str());
  logRadio();
  logPowerSave();
  return true;
}

LinkState link(uint32_t nowMs) {
  if (WiFi.status() == WL_CONNECTED) {
    return LinkState::UP;
  }

  if (connected) {
    return reconnecting ? LinkState::LOST : LinkState::UP;
  }

  // a board without credentials waits in the portal, one with them is still trying or gave up
  if (!credentialsSaved) {
    return LinkState::PORTAL;
  }

  return nowMs - beganMs < kConnectTimeoutMs ? LinkState::CONNECTING : LinkState::LOST;
}

String ipString() {
  return WiFi.localIP().toString();
}

int8_t rssi() {
  return static_cast<int8_t>(WiFi.RSSI());
}

void forget() {
  manager.resetSettings();
}

}
