#include "state/State.h"

#include "board/boardConfig.h"

namespace {

/**
 * Returns whether a state keeps the matrix while the clock, usage or off view is on.
 * @param {PunktState} state - state to test
 * @param {bool} flashing - true while the no-session glitch flash runs
 * @returns {bool} - true for the states that must be seen and for off, which stays dark
 */
bool breaksThroughView(PunktState state, bool flashing) {
  if (state == PunktState::OFF) {
    // an empty table is dark, the no-session flash is the one picture that replaces it
    return !flashing;
  }

  return state == PunktState::ATTENTION || state == PunktState::ERROR ||
         state == PunktState::LOADING;
}

/**
 * Returns whether a state that breaks through the view keeps the side clock beside it.
 * @param {PunktState} state - state that breaks through the view
 * @returns {bool} - true for ATTENTION and ERROR on a board with a clock half
 */
bool splitsBreakThrough(PunktState state) {
  return Board::kHasClockHalf && (state == PunktState::ATTENTION || state == PunktState::ERROR);
}

}

const char* StateMachine::animationName() const {
  return animation_->name();
}

bool StateMachine::showsClock() const {
  return animation_ == &clock_ || animation_ == &wideClock_;
}

void StateMachine::setResetProgress(int16_t pixels, uint32_t) {
  const bool resetting = pixels >= 0;

  if (resetting) {
    resetFill_.setProgress(static_cast<uint8_t>(pixels));
  }

  if (resetting == resetting_) {
    return;
  }

  resetting_ = resetting;
  refresh(nullptr, resetting ? "reset hold" : "reset released");
}

void StateMachine::setOtaProgress(int16_t pixels, uint32_t) {
  const bool updating = pixels >= 0;

  if (updating) {
    otaSpiral_.setProgress(static_cast<uint8_t>(pixels));
  }

  if (updating == updating_) {
    return;
  }

  updating_ = updating;
  refresh(nullptr, updating ? "ota" : "ota end");
}

void StateMachine::setLink(WifiSetup::LinkState link, uint32_t) {
  if (link == link_) {
    return;
  }

  link_ = link;
  refresh(nullptr, "link");
}

bool StateMachine::systemPicture() const {
  return resetting_ || updating_ || link_ == WifiSetup::LinkState::PORTAL ||
         link_ == WifiSetup::LinkState::LOST;
}

StateMachine::Picture StateMachine::displayAnimationFor(PunktState state, uint8_t activity,
                                                       int8_t animation) {
  if (resetting_) {
    return {resetFill_, false};
  }

  // a link the board cannot use is worth more than any picture, only a dark matrix stays dark
  if (link_ == WifiSetup::LinkState::PORTAL || link_ == WifiSetup::LinkState::LOST) {
    if (view_ == View::OFF) {
      return {off_, false};
    }

    if (link_ == WifiSetup::LinkState::PORTAL) {
      return {wifiSearch_, false};
    }

    return {wifiLost_, false};
  }

  if (updating_) {
    return {otaSpiral_, false};
  }

  // a preview outranks every view, the dark matrix of an off view included
  if (preview_ != nullptr) {
    // a previewed clock shows the full face, a split one would put two clocks side by side
    if (preview_ == &clock_ && Board::kHasClockHalf) {
      return {wideClock_, false};
    }

    return {*preview_, Board::kHasClockHalf && !ownsCanvas(*preview_)};
  }

  const bool flashing = noSessionFlashUntilMs_ != 0;

  if (breaksThroughView(state, flashing)) {
    return {animationFor(state, activity, animation), splitsBreakThrough(state)};
  }

  if (flashing) {
    return {glitch_, Board::kHasClockHalf};
  }

  // the gauge only means something while a Claude Code session is registered
  if (view_ == View::USAGE && sessions_.namedCount() > 0) {
    return {usageView_, Board::kHasClockHalf};
  }

  if (view_ == View::CLOCK) {
    return {clockFace(), false};
  }

  if (kHasBatteryView && view_ == View::BATTERY) {
    return {batteryFace_, false};
  }

  if (view_ == View::OFF) {
    return {off_, false};
  }

  // the split idle gauge means a registered session is resting, an empty board rests on the clock
  if (state == PunktState::IDLE && sessions_.namedCount() == 0) {
    return {clockFace(), false};
  }

  return {animationFor(state, activity, animation), Board::kHasClockHalf};
}

bool StateMachine::ownsCanvas(const Animation& animation) const {
  return &animation == &resetFill_ || &animation == &otaSpiral_ || &animation == &wifiSearch_ ||
         &animation == &wifiLost_ || &animation == &off_;
}

Animation& StateMachine::clockFace() {
  if (Board::kHasClockHalf) {
    return wideClock_;
  }

  return clock_;
}
