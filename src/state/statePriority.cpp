#include "state/statePriority.h"

uint8_t statePriority(PunktState state) {
  switch (state) {
    case PunktState::LOADING:
      return 12;
    case PunktState::ATTENTION:
      return 11;
    case PunktState::ERROR:
      return 10;
    case PunktState::GLITCH:
      return 9;
    case PunktState::DONE:
      return 8;
    case PunktState::WAITING:
      return 7;
    case PunktState::COMPACTING:
      return 6;
    case PunktState::WORKING:
      return 5;
    case PunktState::UNREAD:
      return 3;
    case PunktState::LIMIT:
      return 2;
    case PunktState::IDLE:
      return 1;
    default:
      return 0;
  }
}

bool isBusyState(PunktState state) {
  switch (state) {
    case PunktState::LOADING:
    case PunktState::WORKING:
    case PunktState::ATTENTION:
    case PunktState::DONE:
    case PunktState::ERROR:
    case PunktState::WAITING:
    case PunktState::COMPACTING:
    case PunktState::LIMIT:
    case PunktState::GLITCH:
      return true;
    default:
      return false;
  }
}
