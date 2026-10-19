#include "touch/coinNames.h"

const char* coinName(Coin coin) {
  return coin == Coin::LEFT ? "left" : "right";
}
