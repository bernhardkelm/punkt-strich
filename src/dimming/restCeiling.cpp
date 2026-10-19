#include "dimming/restCeiling.h"

uint8_t restCeiling(uint8_t sunLevel, uint8_t nightLevel, bool resting) {
  if (!resting) {
    return sunLevel;
  }

  return sunLevel < nightLevel ? sunLevel : nightLevel;
}
