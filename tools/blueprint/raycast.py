"""Finds the nearest surface along parallel view rays, for orthographic views."""

import numpy as np

CELL_MM = 1.0


def nearest_depth(points, tris, cell=CELL_MM):
    """
    Casts one view ray per point and returns the depth of the first surface hit.
    Points and triangles are in view space: x, y on the image plane, z the depth
    growing away from the viewer. Triangles are binned on a 2D grid so each point
    is only tested against the triangles overlapping its cell.
    @param {np.ndarray} points - (n, 2) image plane positions
    @param {np.ndarray} tris - (m, 3, 3) triangle corners in view space
    @param {float} cell - Bin size in mm
    @returns {np.ndarray} - (n,) depth of the nearest hit, inf where the ray misses
    """
    result = np.full(len(points), np.inf)
    if len(points) == 0 or len(tris) == 0:
        return result
    origin = np.minimum(points.min(0), tris[:, :, :2].min((0, 1))) - cell
    lo = np.floor((tris[:, :, :2].min(1) - origin) / cell).astype(int)
    hi = np.floor((tris[:, :, :2].max(1) - origin) / cell).astype(int)
    pc = np.floor((points - origin) / cell).astype(int)
    width = int(max(hi[:, 0].max(), pc[:, 0].max())) + 1

    tri_cells, tri_ids = _expand_bins(lo, hi, width)
    order = np.argsort(tri_cells, kind="stable")
    tri_cells, tri_ids = tri_cells[order], tri_ids[order]

    point_cells = pc[:, 1] * width + pc[:, 0]
    porder = np.argsort(point_cells, kind="stable")
    sorted_cells = point_cells[porder]
    unique_cells, pstart = np.unique(sorted_cells, return_index=True)
    pend = np.append(pstart[1:], len(sorted_cells))
    tstart = np.searchsorted(tri_cells, unique_cells, "left")
    tend = np.searchsorted(tri_cells, unique_cells, "right")

    for c in range(len(unique_cells)):
        if tend[c] == tstart[c]:
            continue
        pidx = porder[pstart[c]:pend[c]]
        tidx = tri_ids[tstart[c]:tend[c]]
        result[pidx] = _depth_block(points[pidx], tris[tidx])
    return result


def _expand_bins(lo, hi, width):
    """
    Lists every (cell, triangle) pair for the triangles' bounding boxes.
    @param {np.ndarray} lo - (m, 2) lower cell indices
    @param {np.ndarray} hi - (m, 2) upper cell indices
    @param {int} width - Cells per grid row
    @returns {tuple} - Flat cell ids and the matching triangle ids
    """
    nx = hi[:, 0] - lo[:, 0] + 1
    ny = hi[:, 1] - lo[:, 1] + 1
    counts = nx * ny
    tri_ids = np.repeat(np.arange(len(lo)), counts)
    offsets = np.arange(counts.sum()) - np.repeat(np.cumsum(counts) - counts, counts)
    nxr = np.repeat(nx, counts)
    cx = lo[tri_ids, 0] + offsets % nxr
    cy = lo[tri_ids, 1] + offsets // nxr
    return cy * width + cx, tri_ids


def _depth_block(p, t, chunk=4096):
    """
    Tests points against triangles with 2D barycentrics and keeps the nearest depth.
    @param {np.ndarray} p - (n, 2) points
    @param {np.ndarray} t - (k, 3, 3) triangles in view space
    @param {int} chunk - Points per vectorized batch
    @returns {np.ndarray} - (n,) nearest depth or inf
    """
    a, b, c = t[:, 0], t[:, 1], t[:, 2]
    v0 = (b - a)[:, :2]
    v1 = (c - a)[:, :2]
    den = v0[:, 0] * v1[:, 1] - v1[:, 0] * v0[:, 1]
    valid = np.abs(den) > 1e-12
    a, b, c, v0, v1, den = a[valid], b[valid], c[valid], v0[valid], v1[valid], den[valid]
    out = np.full(len(p), np.inf)
    if len(a) == 0:
        return out
    for s in range(0, len(p), chunk):
        q = p[s:s + chunk, None, :] - a[None, :, :2]
        u = (q[..., 0] * v1[:, 1] - v1[:, 0] * q[..., 1]) / den
        v = (v0[:, 0] * q[..., 1] - q[..., 0] * v0[:, 1]) / den
        inside = (u >= -1e-9) & (v >= -1e-9) & (u + v <= 1 + 1e-9)
        z = a[:, 2] + u * (b[:, 2] - a[:, 2]) + v * (c[:, 2] - a[:, 2])
        out[s:s + chunk] = np.where(inside, z, np.inf).min(1)
    return out
