"""Writes the SVG elements of the drawing: model lines, LEDs, dimensions and labels."""

import numpy as np

FONT = "ui-monospace, SFMono-Regular, Menlo, monospace"
LINE_PX = 1.0
DETAIL_PX = 0.6
DIM_FONT_PX = 10
LABEL_FONT_PX = 11
DIM_OFFSET_PX = 9
EXT_GAP_PX = 2
EXT_OVERSHOOT_PX = 3
TICK_PX = 2.2
CHAR_PX = 0.62 * DIM_FONT_PX
LED_RADIUS_MM = 0.8
LED_OFF_OPACITY = 0.14
HIDDEN_OPACITY = 0.45
HIDDEN_DASH = "1.6 1.4"


def num(v):
    """
    Formats a coordinate with one decimal and no trailing zero.
    @param {float} v - Value
    @returns {str} - Compact text
    """
    text = f"{v:.1f}"
    if text.endswith(".0"):
        text = text[:-2]
    return "0" if text == "-0" else text


class Placement:
    """Maps view coordinates in mm, y up, to SVG pixels."""

    def __init__(self, view, x, y, scale):
        """
        Anchors a view's bounding box with its top left corner at a pixel position.
        @param {View} view - The view to place
        @param {float} x - Left edge in px
        @param {float} y - Top edge in px
        @param {float} scale - Pixels per mm
        @returns {void}
        """
        self.view, self.x, self.y, self.scale = view, x, y, scale

    def map(self, pts):
        """
        Converts view points to pixels.
        @param {np.ndarray} pts - (..., 2) points in mm
        @returns {np.ndarray} - (..., 2) points in px
        """
        lo, hi = self.view.lo, self.view.hi
        pts = np.asarray(pts, float)
        return np.stack(
            [self.x + (pts[..., 0] - lo[0]) * self.scale, self.y + (hi[1] - pts[..., 1]) * self.scale], -1
        )

    @property
    def width(self):
        """
        Returns the drawn width.
        @returns {float} - Width in px
        """
        return (self.view.hi[0] - self.view.lo[0]) * self.scale

    @property
    def height(self):
        """
        Returns the drawn height.
        @returns {float} - Height in px
        """
        return (self.view.hi[1] - self.view.lo[1]) * self.scale

    @property
    def right(self):
        """
        Returns the right edge.
        @returns {float} - x in px
        """
        return self.x + self.width

    @property
    def bottom(self):
        """
        Returns the bottom edge.
        @returns {float} - y in px
        """
        return self.y + self.height


def path_data(lines):
    """
    Serialises polylines in px as SVG path data with relative moves.
    @param {list} lines - (p, 2) polylines in px
    @returns {str} - Path data
    """
    parts = []
    for line in lines:
        q = np.round(np.asarray(line) * 10).astype(int)
        steps = np.diff(q, axis=0)
        steps = steps[np.any(steps != 0, axis=1)]
        if len(steps) == 0:
            continue
        cmd = [f"M{num(q[0, 0] / 10)} {num(q[0, 1] / 10)}l"]
        cmd.append(" ".join(f"{num(dx / 10)} {num(dy / 10)}" for dx, dy in steps))
        parts.append("".join(cmd))
    return "".join(parts).replace(" -", "-")


def model_lines(placement, palette):
    """
    Draws the visible and hidden model lines of a placed view.
    @param {Placement} placement - Placed view
    @param {dict} palette - Colours
    @returns {list} - SVG elements
    """
    out = []
    view = placement.view
    if view.hidden:
        d = path_data([placement.map(l) for l in view.hidden])
        out.append(
            f'<path d="{d}" fill="none" stroke="{palette["ink"]}" stroke-width="{DETAIL_PX}" '
            f'stroke-dasharray="{HIDDEN_DASH}" stroke-opacity="{HIDDEN_OPACITY}"/>'
        )
    d = path_data([placement.map(l) for l in view.lines])
    out.append(
        f'<path d="{d}" fill="none" stroke="{palette["ink"]}" stroke-width="{LINE_PX}" '
        f'stroke-linejoin="round" stroke-linecap="round"/>'
    )
    return out


def leds(placement, device, palette):
    """
    Draws the LED pattern on the front face, mapped into the view with one affine transform.
    @param {Placement} placement - Placed front or axonometric view
    @param {Device} device - Device with LED centres and the frame to show
    @param {dict} palette - Colours
    @returns {list} - SVG elements
    """
    r = placement.view.rotation
    origin = placement.map(np.array([r[0, 2], r[1, 2]]) * device.face_z)
    ex = placement.map(np.array([r[0, 0], r[1, 0]]) + np.array([r[0, 2], r[1, 2]]) * device.face_z) - origin
    ey = placement.map(np.array([r[0, 1], r[1, 1]]) + np.array([r[0, 2], r[1, 2]]) * device.face_z) - origin
    matrix = " ".join(num(v) if i > 3 else f"{v:.4f}" for i, v in enumerate((*ex, *ey, *origin)))
    rad = num(LED_RADIUS_MM)
    lit, dark = [], []
    for (row, col), level in np.ndenumerate(device.frame):
        x, y = device.leds[row, col]
        circle = f'<circle cx="{num(x)}" cy="{num(y)}" r="{rad}"'
        if level > 0:
            opacity = "" if level > 0.9 else f' fill-opacity="{level:.2f}"'
            lit.append(f"{circle}{opacity}/>")
        else:
            dark.append(f"{circle}/>")
    return [
        f'<g transform="matrix({matrix})">'
        f'<g fill="{palette["ink"]}" fill-opacity="{LED_OFF_OPACITY}">{"".join(dark)}</g>'
        f'<g fill="{palette["led"]}">{"".join(lit)}</g></g>'
    ]


def _text(x, y, value, palette, rotate=False):
    """
    Writes a centred dimension value.
    @param {float} x - Centre x in px
    @param {float} y - Centre y in px
    @param {str} value - Text
    @param {dict} palette - Colours
    @param {bool} rotate - Turns the text to read upwards
    @returns {str} - SVG text element
    """
    transform = f' transform="rotate(-90 {num(x)} {num(y)})"' if rotate else ""
    return (
        f'<text x="{num(x)}" y="{num(y + DIM_FONT_PX * 0.35)}" text-anchor="middle"'
        f'{transform}>{value}</text>'
    )


def _tick(x, y):
    """
    Returns path data for a symmetric x mark centred on a dimension end.
    @param {float} x - x in px
    @param {float} y - y in px
    @returns {str} - Path data
    """
    return (
        f"M{num(x - TICK_PX)} {num(y + TICK_PX)}L{num(x + TICK_PX)} {num(y - TICK_PX)}"
        f"M{num(x - TICK_PX)} {num(y - TICK_PX)}L{num(x + TICK_PX)} {num(y + TICK_PX)}"
    )


def dim_horizontal(x0, x1, y_edge, value, palette):
    """
    Draws a width dimension below an edge, with extension lines, ticks and the value in a gap.
    @param {float} x0 - Left end in px
    @param {float} x1 - Right end in px
    @param {float} y_edge - Bottom edge of the measured view in px
    @param {str} value - Label
    @param {dict} palette - Colours
    @returns {list} - SVG elements
    """
    y = y_edge + DIM_OFFSET_PX
    mid = (x0 + x1) / 2
    half = len(value) * CHAR_PX / 2 + 3
    d = (
        f"M{num(x0)} {num(y_edge + EXT_GAP_PX)}V{num(y + EXT_OVERSHOOT_PX)}"
        f"M{num(x1)} {num(y_edge + EXT_GAP_PX)}V{num(y + EXT_OVERSHOOT_PX)}"
        f"M{num(x0)} {num(y)}H{num(mid - half)}M{num(mid + half)} {num(y)}H{num(x1)}"
        f"{_tick(x0, y)}{_tick(x1, y)}"
    )
    return [f'<path d="{d}"/>', _text(mid, y, value, palette)]


def dim_vertical(y0, y1, x_edge, value, palette, side=-1):
    """
    Draws a height dimension beside an edge, with extension lines, ticks and the value in a gap.
    @param {float} y0 - Top end in px
    @param {float} y1 - Bottom end in px
    @param {float} x_edge - Measured edge in px
    @param {str} value - Label
    @param {dict} palette - Colours
    @param {int} side - -1 places it left of the edge, 1 right of it
    @returns {list} - SVG elements
    """
    x = x_edge + side * DIM_OFFSET_PX
    mid = (y0 + y1) / 2
    half = len(value) * CHAR_PX / 2 + 3
    d = (
        f"M{num(x_edge + side * EXT_GAP_PX)} {num(y0)}H{num(x + side * EXT_OVERSHOOT_PX)}"
        f"M{num(x_edge + side * EXT_GAP_PX)} {num(y1)}H{num(x + side * EXT_OVERSHOOT_PX)}"
        f"M{num(x)} {num(y0)}V{num(mid - half)}M{num(x)} {num(mid + half)}V{num(y1)}"
        f"{_tick(x, y0)}{_tick(x, y1)}"
    )
    return [f'<path d="{d}"/>', _text(x, mid, value, palette, rotate=True)]


def dim_group(elements, palette):
    """
    Wraps dimension elements in one group carrying their shared style.
    @param {list} elements - SVG elements
    @param {dict} palette - Colours
    @returns {str} - SVG group
    """
    return (
        f'<g fill="none" stroke="{palette["ink"]}" stroke-width="{DETAIL_PX}">'
        f'<g>{"".join(e for e in elements if e.startswith("<path"))}</g>'
        f'<g fill="{palette["ink"]}" stroke="none" font-size="{DIM_FONT_PX}">'
        f'{"".join(e for e in elements if e.startswith("<text"))}</g></g>'
    )


def label(x, y, text, palette):
    """
    Writes a device name.
    @param {float} x - Left edge in px
    @param {float} y - Baseline in px
    @param {str} text - Name
    @param {dict} palette - Colours
    @returns {str} - SVG text element
    """
    return f'<text x="{num(x)}" y="{num(y)}" font-size="{LABEL_FONT_PX}" fill="{palette["label"]}">{text}</text>'


def size_text(mm):
    """
    Formats a dimension value in mm.
    @param {float} mm - Length
    @returns {str} - Label
    """
    return num(mm)
