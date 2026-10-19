"""Reads LED frames back from the matrix renders that tools/render writes."""

import numpy as np
from PIL import Image, ImageSequence

# board geometry and colours of tools/render/gif.py, the renders are drawn at 1x
PAD = 12
CELL = 24
ROWS = 8
OFF = np.array([42, 38, 35], float)
ON = np.array([232, 121, 43], float)
LIT_MIN = 0.12


def _grid(image):
    """
    Samples the LED centres of one rendered board.
    @param {Image} image - Rendered board
    @returns {np.ndarray} - (8, columns) brightness, 0 unlit to 1 fully lit
    """
    rgb = np.asarray(image.convert("RGB"), float)
    columns = (image.width - 2 * PAD) // CELL
    ys = PAD + np.arange(ROWS) * CELL + CELL // 2
    xs = PAD + np.arange(columns) * CELL + CELL // 2
    centre = rgb[ys][:, xs]
    # summed over the channels, the blue of the orange barely rises above OFF
    level = (centre - OFF).sum(-1) / (ON - OFF).sum()
    level = np.clip(level, 0.0, 1.0)
    return np.where(level >= LIT_MIN, level, 0.0)


def still(path):
    """
    Reads the LED grid of a still render.
    @param {str} path - PNG written by tools/render
    @returns {np.ndarray} - (8, columns) brightness
    """
    with Image.open(path) as image:
        return _grid(image)


def densest(path):
    """
    Reads the frame of an animation render that lights the most LEDs.
    @param {str} path - GIF written by tools/render
    @returns {np.ndarray} - (8, columns) brightness
    """
    with Image.open(path) as image:
        grids = [_grid(frame) for frame in ImageSequence.Iterator(image)]
    return max(grids, key=lambda g: (g > 0).sum())
