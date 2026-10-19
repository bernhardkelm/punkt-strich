"""Describes a device: its meshes, its overall size and where its LEDs sit."""

from dataclasses import dataclass

import numpy as np

from mesh import load_3mf

MATRIX_MM = 20.3
LED_PITCH_MM = 2.5
LED_ROWS = 8
POCKET_MIN_AREA = 300.0


@dataclass
class Device:
    name: str
    meshes: list
    screen_name: str
    lo: np.ndarray  # bounding box minimum, mm
    hi: np.ndarray  # bounding box maximum, mm
    leds: np.ndarray  # (8, columns, 2) LED centres on the front face, world x and y
    face_z: float  # world z of the front face
    frame: np.ndarray  # (8, columns) brightness

    @property
    def size(self):
        """
        Returns the overall width, height and depth as drawn.
        @returns {np.ndarray} - x, y and z extent in mm
        """
        return self.hi - self.lo


def load_device(name, path, screen_name, frame):
    """
    Loads a device model and locates its LED matrices from the pocket behind the screen face.
    @param {str} name - Label of the device
    @param {str} path - 3MF file with the case and the screen plate
    @param {str} screen_name - Object name of the screen plate
    @param {np.ndarray} frame - (8, columns) brightness to show
    @returns {Device} - The device
    """
    meshes = load_3mf(path)
    allv = np.concatenate([m.vertices for m in meshes.values()])
    lo, hi = allv.min(0), allv.max(0)
    centre, width, face_z = matrix_pocket(meshes[screen_name])
    count = max(1, round(width / MATRIX_MM))
    leds = led_grid(centre, width, count)
    if frame.shape != leds.shape[:2]:
        raise ValueError(f"{name}: frame {frame.shape} does not match {leds.shape[:2]} LEDs")
    return Device(name, list(meshes.values()), screen_name, lo, hi, leds, face_z, frame)


def matrix_pocket(screen):
    """
    Finds the largest back facing floor inside the screen plate, the pocket the matrices sit in.
    @param {Mesh} screen - Screen plate mesh, its front face points to +z
    @returns {tuple} - Pocket centre (x, y), pocket width in mm and the front face z
    """
    tri = screen.vertices[screen.faces]
    area = np.linalg.norm(np.cross(tri[:, 1] - tri[:, 0], tri[:, 2] - tri[:, 0]), axis=1) / 2
    z = np.round(tri[:, :, 2].mean(1), 2)
    back = screen.normals[:, 2] < -0.99
    face_z = screen.vertices[:, 2].max()
    floors = [(area[back & (z == level)].sum(), level) for level in np.unique(z[back])]
    floors = [(a, level) for a, level in floors if a >= POCKET_MIN_AREA and level < face_z - 0.1]
    _, level = max(floors, key=lambda f: f[1])
    pts = tri[back & (z == level)].reshape(-1, 3)
    lo, hi = pts[:, :2].min(0), pts[:, :2].max(0)
    return (lo + hi) / 2, hi[0] - lo[0], face_z


def led_grid(centre, width, count):
    """
    Lays out the LED centres of matrices sharing the pocket side by side, row 0 on top.
    @param {np.ndarray} centre - Pocket centre (x, y)
    @param {float} width - Pocket width in mm
    @param {int} count - Number of 8x8 matrices
    @returns {np.ndarray} - (8, 8 * count, 2) LED centres
    """
    offsets = (np.arange(LED_ROWS) - (LED_ROWS - 1) / 2) * LED_PITCH_MM
    xs = []
    for m in range(count):
        mx = centre[0] - width / 2 + width / count * (m + 0.5)
        xs.extend(mx + offsets)
    gx, gy = np.meshgrid(np.array(xs), centre[1] - offsets)
    return np.stack([gx, gy], -1)
