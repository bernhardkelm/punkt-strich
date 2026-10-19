"""
Renders the README header: line drawings of punkt and strich in front, side and iso view.
Needs numpy and Pillow, see requirements.txt. Run from anywhere: python3 -I tools/blueprint/blueprint.py
"""

import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import numpy as np  # noqa: E402

import frames  # noqa: E402
import layout  # noqa: E402
import views  # noqa: E402
from device import load_device  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "docs" / "blueprint"
DOCS = ROOT / "docs"

LIGHT = {"ink": "#1f2328", "label": "#656d76", "led": "#e8792b"}
DARK = {"ink": "#e6edf3", "label": "#8b949e", "led": "#e8792b"}


def load_devices():
    """
    Loads both devices with the frames they show.
    @returns {list} - (Device, views) pairs, punkt first
    """
    punkt_frame = frames.densest(DOCS / "animations" / "code.gif")
    clock = frames.still(DOCS / "strich" / "split-digits.png")[:, :8]
    strich_frame = np.hstack([clock, frames.densest(DOCS / "animations" / "tetris.gif")])
    devices = [
        load_device("punkt", ROOT / "hardware" / "punkt_case.3mf", "Screen", punkt_frame),
        load_device("strich", ROOT / "hardware" / "strich_case.3mf", "Strich screen", strich_frame),
    ]
    return [
        (
            d,
            {
                "front": views.project(d, "front", views.FRONT, hidden=(d.screen_name,)),
                "side": views.project(d, "side", views.SIDE, hidden=(d.screen_name,)),
                "iso": views.project(d, "iso", views.ISO),
            },
        )
        for d in devices
    ]


def main():
    """
    Writes both layouts in a light and a dark variant.
    @returns {void}
    """
    devices = load_devices()
    OUT.mkdir(parents=True, exist_ok=True)
    for name, build in (("header-a", layout.one_row), ("header-b", layout.two_rows)):
        for suffix, palette in (("", LIGHT), ("-dark", DARK)):
            path = OUT / f"{name}{suffix}.svg"
            path.write_text(build(devices, palette))
            print(f"{path.relative_to(ROOT)} {path.stat().st_size // 1024} KB")


if __name__ == "__main__":
    main()
