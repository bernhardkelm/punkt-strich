#include "clockFaces/BatteryFace.h"

#include "clockFaces/batteryDigits.h"
#include "clockFaces/batteryFill.h"
#include "clockFaces/batteryOutline.h"
#include "clockFaces/canvasFrame.h"
#include "clockFaces/chargeBolt.h"
#include "power/VbusSense.h"

const char* BatteryFace::name() const {
  return "battery";
}

void BatteryFace::begin(Display& display) {
  shown_ = currentLook();
  draw(display, shown_);
}

void BatteryFace::tick(Display& display, uint32_t) {
  const Look look = currentLook();

  if (look.usb == shown_.usb && look.percent == shown_.percent) {
    return;
  }

  shown_ = look;
  draw(display, look);
}

BatteryFace::Look BatteryFace::currentLook() {
  Look look;
  look.usb = VbusSense::usbPresent();

  if (!look.usb) {
    look.percent = static_cast<int8_t>(VbusSense::percent());
  }

  return look;
}

Frame BatteryFace::frameFor(const Look& look) {
  Frame frame = batteryOutline();
  Frame inside;

  if (look.usb) {
    inside = chargeBolt();
  } else if (look.percent >= 0) {
    const uint8_t percent = static_cast<uint8_t>(look.percent);
    const Frame fill = batteryFill(percent);
    const Frame digits = batteryDigits(percent);

    for (uint8_t y = 0; y < Display::kHeight; y++) {
      inside.rows[y] = static_cast<uint16_t>(fill.rows[y] ^ digits.rows[y]);
    }
  }

  for (uint8_t y = 0; y < Display::kHeight; y++) {
    frame.rows[y] = static_cast<uint16_t>(frame.rows[y] | inside.rows[y]);
  }

  return frame;
}

void BatteryFace::draw(Display& display, const Look& look) {
  drawCanvasFrame(display, frameFor(look));
  display.show();
}
