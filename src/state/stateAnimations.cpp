#include "state/State.h"

#include "board/boardConfig.h"

Animation& StateMachine::animationFor(PunktState state, uint8_t activity, int8_t animation) {
  switch (state) {
    case PunktState::OFF:
      return off_;
    case PunktState::LOADING:
      return loadingOrbit_;
    case PunktState::WORKING:
      return workingAnimation(activity, animation);
    case PunktState::ATTENTION:
      return attention_;
    case PunktState::DONE:
      return done_;
    case PunktState::ERROR:
      return error_;
    case PunktState::WAITING:
      return waiting_;
    case PunktState::COMPACTING:
      return compacting_;
    case PunktState::LIMIT:
      return limit_;
    case PunktState::UNREAD:
      return unread_;
    case PunktState::GLITCH:
      return glitch_;
    case PunktState::IDLE:
    default:
      if (Board::kHasClockHalf) {
        return idleGauge_;
      }

      return clock_;
  }
}

Animation& StateMachine::workingAnimation(uint8_t activity, int8_t animation) {
  const WorkingAnimation selected =
      animation < 0 ? settings_.activityAnimation(static_cast<Activity>(activity))
                    : static_cast<WorkingAnimation>(animation);

  switch (selected) {
    case WorkingAnimation::LIFE:
      return life_;
    case WorkingAnimation::KNIGHT:
      return knight_;
    case WorkingAnimation::ORBIT:
      return orbit_;
    case WorkingAnimation::CODE:
      return codeLines_;
    case WorkingAnimation::RAIN:
      return matrixRain_;
    case WorkingAnimation::SCAN:
      return scan_;
    case WorkingAnimation::PIPES:
      return pipes_;
    case WorkingAnimation::POINTER:
      return pointer_;
    case WorkingAnimation::PONG:
      return pong_;
    case WorkingAnimation::TERMINAL:
      return terminal_;
    case WorkingAnimation::TETRIS:
    default:
      return tetris_;
  }
}
