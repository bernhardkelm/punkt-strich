// Host definitions of the firmware services the strich clock faces read: time, settings, VBUS.
#include "hostServices.h"

#include "network/TimeSync.h"
#include "power/VbusSense.h"
#include "settings/Settings.h"

namespace {

/** Hour every strich picture shows */
constexpr int kRenderHour = 20;
/** Minute every strich picture shows */
constexpr int kRenderMinute = 47;

bool g_usb = false;
int g_percent = -1;

}

void setHostBattery(bool usb, int percent) {
  g_usb = usb;
  g_percent = percent;
}

bool TimeSync::localTime(tm& out) {
  out = tm{};
  out.tm_hour = kRenderHour;
  out.tm_min = kRenderMinute;
  return true;
}

bool VbusSense::usbPresent() {
  return g_usb;
}

int VbusSense::percent() {
  return g_usb ? -1 : g_percent;
}

ClockFace Settings::clockFace() const {
  return clockFace_;
}

void Settings::setClockFace(ClockFace face) {
  clockFace_ = face;
}

SplitClock Settings::splitClock() const {
  return splitClock_;
}

void Settings::setSplitClock(SplitClock style) {
  splitClock_ = style;
}
