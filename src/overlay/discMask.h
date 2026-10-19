#pragma once

#include "Frame.h"

#include <cstdint>

/**
 * Returns the filled disc around the overlay origin, the point between columns 3 and 4 just above
 * row 0 shifted right by originX, measured in doubled coordinates dx = 2(x - originX) - 7,
 * dy = 2y + 1, clipped to the canvas and to the columns firstCol to lastCol.
 * @param {uint16_t} r2 - squared radius in doubled units, pixels at or below it are set
 * @param {uint8_t} originX - columns the origin is shifted right, 0 for the left coin
 * @param {uint8_t} firstCol - first column kept
 * @param {uint8_t} lastCol - last column kept, inclusive
 * @returns {Frame} - frame with the disc pixels set
 */
Frame discMask(uint16_t r2, uint8_t originX = 0, uint8_t firstCol = 0, uint8_t lastCol = 255);

/**
 * Returns the ring between two squared radii around the same origin, the front of the ripple.
 * @param {uint16_t} r2Outer - squared outer radius in doubled units, inclusive
 * @param {uint16_t} r2Inner - squared inner radius in doubled units, exclusive
 * @param {uint8_t} originX - columns the origin is shifted right, 0 for the left coin
 * @param {uint8_t} firstCol - first column kept
 * @param {uint8_t} lastCol - last column kept, inclusive
 * @returns {Frame} - frame with the ring pixels set
 */
Frame ringMask(uint16_t r2Outer, uint16_t r2Inner, uint8_t originX = 0, uint8_t firstCol = 0,
               uint8_t lastCol = 255);

/**
 * Returns the one pixel outline of a shape, its 4-neighbour dilation clipped to the canvas minus
 * the shape itself,
 * limited to the columns firstCol to lastCol.
 * @param {Frame} frame - shape to outline
 * @param {uint8_t} firstCol - first column kept
 * @param {uint8_t} lastCol - last column kept, inclusive
 * @returns {Frame} - frame with the outline pixels set
 */
Frame keyline(const Frame& frame, uint8_t firstCol = 0, uint8_t lastCol = 255);

/**
 * Returns the distance of the farthest corner of the columns firstCol to lastCol from the overlay
 * origin, the radius at which a disc around it covers that whole region.
 * @param {uint8_t} originX - columns the origin is shifted right
 * @param {uint8_t} firstCol - first column of the region
 * @param {uint8_t} lastCol - last column of the region, inclusive
 * @returns {float} - radius in doubled units
 */
float cornerRadius(uint8_t originX, uint8_t firstCol, uint8_t lastCol);

/**
 * Returns a radius in doubled units as the squared radius the masks take.
 * @param {float} radius - radius in doubled units
 * @returns {uint16_t} - squared radius, rounded to the nearest integer
 */
uint16_t squaredRadius(float radius);

/**
 * Returns the pixelwise union of two frames.
 * @param {Frame} a - first frame
 * @param {Frame} b - second frame
 * @returns {Frame} - frame with every pixel set in a or b
 */
Frame merged(const Frame& a, const Frame& b);
