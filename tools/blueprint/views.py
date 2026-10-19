"""Defines the view rotations: rows are the image right, image up and the view direction."""

from dataclasses import dataclass

import numpy as np

from lines import view_lines
from paths import drop_overlaps, polylines

UP = np.array([0.0, 1.0, 0.0])


def look(forward, up=UP):
    """
    Builds an orthographic view rotation looking along a direction.
    @param {np.ndarray} forward - Direction from the viewer into the scene
    @param {np.ndarray} up - World direction that should point up in the image
    @returns {np.ndarray} - (3, 3) rotation, rows right, up, forward
    """
    f = np.asarray(forward, float)
    f = f / np.linalg.norm(f)
    right = np.cross(f, up)
    right /= np.linalg.norm(right)
    image_up = np.cross(right, f)
    return np.array([right, image_up, f])


def axonometric(azimuth_deg, elevation_deg):
    """
    Builds a view from the front right and above, showing the front, the top and the right side.
    @param {float} azimuth_deg - Rotation of the viewer from the front towards the right side
    @param {float} elevation_deg - Height of the viewer above the horizon
    @returns {np.ndarray} - (3, 3) view rotation
    """
    az, el = np.radians(azimuth_deg), np.radians(elevation_deg)
    eye = np.array([np.sin(az) * np.cos(el), np.sin(el), np.cos(az) * np.cos(el)])
    return look(-eye)


FRONT = look([0, 0, -1])
SIDE = look([-1, 0, 0])
ISO = axonometric(30.0, 20.0)


@dataclass
class View:
    name: str
    rotation: np.ndarray
    lines: list  # visible polylines, mm, y up
    hidden: list  # hidden polylines, mm, y up
    lo: np.ndarray  # projected bounding box minimum
    hi: np.ndarray  # projected bounding box maximum


def project(device, name, rotation, hidden=()):
    """
    Draws one orthographic view of a device.
    @param {Device} device - Device to draw
    @param {str} name - View name
    @param {np.ndarray} rotation - View rotation
    @param {tuple} hidden - Names of the meshes whose covered edges are drawn as hidden lines
    @returns {View} - The projected view
    """
    visible, covered = view_lines(device.meshes, rotation, [m for m in device.meshes if m.name in hidden])
    pts = np.concatenate([m.vertices for m in device.meshes]) @ rotation.T
    lines = polylines(visible)
    hidden_lines = polylines(drop_overlaps(covered, visible)) if len(covered) else []
    return View(name, rotation, lines, hidden_lines, pts[:, :2].min(0), pts[:, :2].max(0))
