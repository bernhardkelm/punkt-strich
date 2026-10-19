#pragma once

#include "Display.h"

#include <cstdint>

/** Quarter arcs the Wi-Fi symbol can show around its dot. */
constexpr uint8_t kWifiArcCount = 3;

/**
 * Draws the Wi-Fi symbol, the dot in the bottom left corner and the quarter arcs around it, each
 * arc being the pixels whose rounded distance from the dot matches its radius. The symbol is
 * centred horizontally on the canvas. The caller clears and shows the matrix.
 * @param {Display&} display - matrix to draw on
 * @param {uint8_t} arcs - arcs lit from the inside out, 0 to kWifiArcCount
 * @returns {void}
 */
void drawWifiSymbol(Display& display, uint8_t arcs);
