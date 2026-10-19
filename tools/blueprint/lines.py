"""Extracts the visible line drawing of meshes for one orthographic view."""

import numpy as np

from raycast import nearest_depth

CREASE_DEG = 30.0
FACING_TOL = 1e-6
SAMPLE_MM = 0.06
DEPTH_EPS_MM = 0.03


def candidate_edges(mesh, forward, crease_deg=CREASE_DEG):
    """
    Selects crease, open and silhouette edges of a mesh for a view direction.
    @param {Mesh} mesh - Source mesh
    @param {np.ndarray} forward - Unit view direction, from the viewer into the scene
    @param {float} crease_deg - Minimum dihedral angle that counts as a crease
    @returns {np.ndarray} - (k, 2, 3) edge end points in world space
    """
    f1, f2 = mesh.edge_faces[:, 0], mesh.edge_faces[:, 1]
    open_edge = f2 < 0
    n1 = mesh.normals[f1]
    n2 = mesh.normals[np.where(open_edge, f1, f2)]
    crease = np.einsum("ij,ij->i", n1, n2) < np.cos(np.radians(crease_deg))
    d1, d2 = n1 @ forward, n2 @ forward
    front1, front2 = d1 < -FACING_TOL, d2 < -FACING_TOL
    silhouette = front1 != front2
    keep = crease | silhouette | open_edge
    return mesh.vertices[mesh.edges[keep]]


def split_segments(edges_view, tris_view, step=SAMPLE_MM, eps=DEPTH_EPS_MM):
    """
    Splits edges into covered and uncovered runs, by casting a view ray per sample.
    @param {np.ndarray} edges_view - (k, 2, 3) edges in view space, z is depth
    @param {np.ndarray} tris_view - (m, 3, 3) occluding triangles in view space
    @param {float} step - Sample spacing along an edge in mm
    @param {float} eps - Depth tolerance against self occlusion in mm
    @returns {tuple} - Visible and hidden (s, 2, 2) 2D segments
    """
    a, b = edges_view[:, 0], edges_view[:, 1]
    length = np.linalg.norm((b - a)[:, :2], axis=1)
    keep = length > 1e-4
    a, b, length = a[keep], b[keep], length[keep]
    counts = np.maximum(1, np.ceil(length / step).astype(int))
    edge_id = np.repeat(np.arange(len(a)), counts)
    index = np.arange(counts.sum()) - np.repeat(np.cumsum(counts) - counts, counts)
    t = (index + 0.5) / counts[edge_id]
    pts = a[edge_id] + (b - a)[edge_id] * t[:, None]
    hit = nearest_depth(pts[:, :2], tris_view)
    visible = hit >= pts[:, 2] - eps

    starts = np.cumsum(counts) - counts
    return (
        _runs(a[:, :2], b[:, :2], visible, starts, counts),
        _runs(a[:, :2], b[:, :2], ~visible, starts, counts),
    )


def _runs(a, b, mask, starts, counts):
    """
    Collects the sub segments of each edge where consecutive samples are set in a mask.
    @param {np.ndarray} a - (k, 2) edge starts
    @param {np.ndarray} b - (k, 2) edge ends
    @param {np.ndarray} mask - Flat per sample flags
    @param {np.ndarray} starts - First sample index per edge
    @param {np.ndarray} counts - Samples per edge
    @returns {np.ndarray} - (s, 2, 2) segments
    """
    out = []
    for e in range(len(a)):
        n = counts[e]
        run = mask[starts[e]:starts[e] + n]
        if not run.any():
            continue
        if run.all():
            out.append((a[e], b[e]))
            continue
        flips = np.flatnonzero(np.diff(np.concatenate([[0], run.astype(int), [0]])))
        for s, f in zip(flips[::2], flips[1::2]):
            out.append((a[e] + (b[e] - a[e]) * (s / n), a[e] + (b[e] - a[e]) * (f / n)))
    return np.array(out).reshape(-1, 2, 2)


def view_lines(meshes, rotation, hidden_from=()):
    """
    Returns the visible 2D segments of a set of meshes seen through a rotation, and the
    hidden segments of selected meshes.
    @param {list} meshes - Mesh objects that also occlude each other
    @param {np.ndarray} rotation - (3, 3) rows are view right, up and forward
    @param {tuple} hidden_from - Meshes whose covered edges are returned as hidden lines
    @returns {tuple} - Visible and hidden (s, 2, 2) segments in mm, y up
    """
    forward = rotation[2]
    tris = np.concatenate([m.vertices[m.faces] for m in meshes]) @ rotation.T
    edges = np.concatenate([candidate_edges(m, forward) for m in meshes]) @ rotation.T
    visible, _ = split_segments(edges, tris)
    hidden = np.zeros((0, 2, 2))
    if hidden_from:
        own = np.concatenate([candidate_edges(m, forward) for m in hidden_from]) @ rotation.T
        _, hidden = split_segments(own, tris)
    return visible, hidden
