"""Arranges the three views of each device into the header drawings."""

import drawing as dr
from drawing import Placement

MARGIN_PX = 4
DIM_ZONE_PX = dr.DIM_OFFSET_PX + dr.DIM_FONT_PX * 0.6 + 2
LABEL_ZONE_PX = dr.LABEL_FONT_PX + 8
VIEW_GAP_PX = 22
ISO_GAP_PX = 26
GROUP_GAP_PX = 44
ROW_GAP_PX = 18


def device_elements(device, views, places, palette):
    """
    Draws one device: lines of all views, the LEDs, the dimensions and its name.
    @param {Device} device - Device
    @param {dict} views - View name mapped to View
    @param {dict} places - View name mapped to Placement
    @param {dict} palette - Colours
    @returns {list} - SVG elements
    """
    front, side, iso = places["front"], places["side"], places["iso"]
    width, height, depth = device.size
    out = []
    for place in places.values():
        out += dr.model_lines(place, palette)
    out += dr.leds(front, device, palette)
    out += dr.leds(iso, device, palette)
    dims = []
    dims += dr.dim_horizontal(front.x, front.right, front.bottom, dr.size_text(width), palette)
    dims += dr.dim_vertical(front.y, front.bottom, front.x, dr.size_text(height), palette)
    dims += dr.dim_horizontal(side.x, side.right, side.bottom, dr.size_text(depth), palette)
    out.append(dr.dim_group(dims, palette))
    out.append(dr.label(front.x, front.y - 8, device.name, palette))
    return out


def _svg(width, height, title, body):
    """
    Wraps the drawing in an SVG root with a fixed pixel size.
    @param {float} width - Width in px
    @param {float} height - Height in px
    @param {str} title - Accessible title
    @param {list} body - SVG elements
    @returns {str} - SVG document
    """
    w, h = round(width), round(height)
    return (
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{w}" height="{h}" viewBox="0 0 {w} {h}" '
        f'role="img" font-family="{dr.FONT}">\n<title>{title}</title>\n' + "\n".join(body) + "\n</svg>\n"
    )


def _row_top(views, scale):
    """
    Returns the space a row needs above its common baseline.
    @param {dict} views - View name mapped to View
    @param {float} scale - Pixels per mm
    @returns {float} - Height in px
    """
    heights = {k: (v.hi[1] - v.lo[1]) * scale for k, v in views.items()}
    return max(max(heights.values()), heights["front"] + LABEL_ZONE_PX)


def _place_row(views, xs, base, scale):
    """
    Places the views of one device on a shared baseline.
    @param {dict} views - View name mapped to View
    @param {dict} xs - View name mapped to its left edge in px
    @param {float} base - Baseline in px
    @param {float} scale - Pixels per mm
    @returns {dict} - View name mapped to Placement
    """
    return {
        k: Placement(v, xs[k], base - (v.hi[1] - v.lo[1]) * scale, scale) for k, v in views.items()
    }


def _widths(views):
    """
    Returns the drawn widths of a device's views in mm.
    @param {dict} views - View name mapped to View
    @returns {dict} - View name mapped to width
    """
    return {k: v.hi[0] - v.lo[0] for k, v in views.items()}


def _columns(widths, x, scale):
    """
    Returns the left edges of the iso, front and side view, in that order from the left, with
    the height dimension of the front view between iso and front.
    @param {dict} widths - View name mapped to its width in mm
    @param {float} x - Left edge of the group in px
    @param {float} scale - Pixels per mm
    @returns {dict} - View name mapped to its left edge in px
    """
    xs = {"iso": x}
    xs["front"] = xs["iso"] + widths["iso"] * scale + ISO_GAP_PX + DIM_ZONE_PX
    xs["side"] = xs["front"] + widths["front"] * scale + VIEW_GAP_PX
    return xs


def one_row(devices, palette, target_width=880):
    """
    Lays out all devices in one row: iso, front and side of each, on one baseline.
    @param {list} devices - (Device, views) pairs
    @param {dict} palette - Colours
    @param {float} target_width - Total width in px
    @returns {str} - SVG document
    """
    per_group = DIM_ZONE_PX + VIEW_GAP_PX + ISO_GAP_PX
    fixed = 2 * MARGIN_PX + len(devices) * per_group + (len(devices) - 1) * GROUP_GAP_PX
    mm = sum(sum(_widths(v).values()) for _, v in devices)
    scale = (target_width - fixed) / mm
    top = max(_row_top(v, scale) for _, v in devices)
    base = MARGIN_PX + top
    body = []
    x = MARGIN_PX
    for device, views in devices:
        w = _widths(views)
        xs = _columns(w, x, scale)
        body += device_elements(device, views, _place_row(views, xs, base, scale), palette)
        x = xs["side"] + w["side"] * scale + GROUP_GAP_PX
    height = base + DIM_ZONE_PX + MARGIN_PX
    return _svg(target_width, height, "punkt and strich, front, side and iso", body)


def two_rows(devices, palette, target_width=800):
    """
    Lays out one row per device with the iso, front and side columns aligned.
    @param {list} devices - (Device, views) pairs
    @param {dict} palette - Colours
    @param {float} target_width - Total width in px
    @returns {str} - SVG document
    """
    cols = {k: max(_widths(v)[k] for _, v in devices) for k in ("front", "side", "iso")}
    fixed = 2 * MARGIN_PX + DIM_ZONE_PX + VIEW_GAP_PX + ISO_GAP_PX
    scale = (target_width - fixed) / sum(cols.values())
    xs = _columns(cols, MARGIN_PX, scale)
    body = []
    y = MARGIN_PX
    for device, views in devices:
        base = y + _row_top(views, scale)
        w = _widths(views)
        centred = {k: xs[k] + (cols[k] - w[k]) * scale / 2 for k in xs}
        body += device_elements(device, views, _place_row(views, centred, base, scale), palette)
        y = base + DIM_ZONE_PX + ROW_GAP_PX
    height = y - ROW_GAP_PX + MARGIN_PX
    return _svg(target_width, height, "punkt and strich, front, side and iso", body)
