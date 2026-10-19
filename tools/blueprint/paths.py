"""Merges, chains and simplifies 2D line segments into compact polylines."""

from collections import defaultdict

import numpy as np

ANGLE_BIN = 0.003
OFFSET_BIN_MM = 0.02
GAP_MM = 0.03
KEY_MM = 0.01
SIMPLIFY_MM = 0.03


def merge_collinear(segments):
    """
    Unions overlapping or touching segments that lie on the same straight line.
    @param {np.ndarray} segments - (s, 2, 2) segments
    @returns {np.ndarray} - (r, 2, 2) merged segments
    """
    d = segments[:, 1] - segments[:, 0]
    length = np.linalg.norm(d, axis=1)
    segments, d, length = segments[length > 1e-6], d[length > 1e-6], length[length > 1e-6]
    theta = np.mod(np.arctan2(d[:, 1], d[:, 0]), np.pi)
    theta[theta > np.pi - ANGLE_BIN / 2] = 0.0
    groups = defaultdict(list)
    for i, th in enumerate(theta):
        u = np.array([np.cos(th), np.sin(th)])
        n = np.array([-u[1], u[0]])
        offset = segments[i, 0] @ n
        key = (round(th / ANGLE_BIN), round(offset / OFFSET_BIN_MM))
        s0, s1 = sorted((segments[i, 0] @ u, segments[i, 1] @ u))
        groups[key].append((s0, s1, offset, th))
    out = []
    for items in groups.values():
        th = np.mean([it[3] for it in items])
        offset = np.mean([it[2] for it in items])
        u = np.array([np.cos(th), np.sin(th)])
        n = np.array([-u[1], u[0]])
        items.sort()
        cur0, cur1 = items[0][0], items[0][1]
        for s0, s1, _, _ in items[1:]:
            if s0 <= cur1 + GAP_MM:
                cur1 = max(cur1, s1)
            else:
                out.append((u * cur0 + n * offset, u * cur1 + n * offset))
                cur0, cur1 = s0, s1
        out.append((u * cur0 + n * offset, u * cur1 + n * offset))
    return np.array(out).reshape(-1, 2, 2)


def chain(segments):
    """
    Joins segments sharing end points into polylines, walking through degree two nodes.
    @param {np.ndarray} segments - (s, 2, 2) segments
    @returns {list} - Polylines as (p, 2) arrays
    """
    keys = [tuple(np.round(p / KEY_MM).astype(int)) for p in segments.reshape(-1, 2)]
    point = {}
    adjacency = defaultdict(list)
    for i in range(len(segments)):
        ka, kb = keys[2 * i], keys[2 * i + 1]
        if ka == kb:
            continue
        point[ka], point[kb] = segments[i, 0], segments[i, 1]
        adjacency[ka].append((kb, i))
        adjacency[kb].append((ka, i))
    used = set()
    lines = []

    def walk(start):
        path = [start]
        node = start
        while True:
            nxt = [(k, i) for k, i in adjacency[node] if i not in used]
            if not nxt:
                return path
            k, i = nxt[0]
            used.add(i)
            path.append(k)
            node = k
            if len(adjacency[node]) != 2:
                return path

    for node in sorted(adjacency, key=lambda k: len(adjacency[k]) == 2):
        while any(i not in used for _, i in adjacency[node]):
            lines.append(np.array([point[k] for k in walk(node)]))
    return lines


def simplify(line, tol=SIMPLIFY_MM):
    """
    Reduces a polyline with the Ramer-Douglas-Peucker algorithm.
    @param {np.ndarray} line - (p, 2) points
    @param {float} tol - Maximum deviation in mm
    @returns {np.ndarray} - Simplified points
    """
    if len(line) < 3:
        return line
    keep = np.zeros(len(line), bool)
    keep[[0, -1]] = True
    stack = [(0, len(line) - 1)]
    while stack:
        i, j = stack.pop()
        if j <= i + 1:
            continue
        a, b = line[i], line[j]
        ab = b - a
        norm = np.linalg.norm(ab)
        rel = line[i + 1:j] - a
        if norm < 1e-12:
            dist = np.linalg.norm(rel, axis=1)
        else:
            dist = np.abs(ab[0] * rel[:, 1] - ab[1] * rel[:, 0]) / norm
        k = int(np.argmax(dist))
        if dist[k] > tol:
            keep[i + 1 + k] = True
            stack += [(i, i + 1 + k), (i + 1 + k, j)]
    return line[keep]


def polylines(segments, min_length=0.05):
    """
    Turns raw view segments into clean polylines.
    @param {np.ndarray} segments - (s, 2, 2) segments in mm
    @param {float} min_length - Drops isolated pieces shorter than this, in mm
    @returns {list} - Simplified polylines
    """
    lines = [simplify(l) for l in chain(merge_collinear(segments))]
    return [l for l in lines if np.linalg.norm(np.diff(l, axis=0), axis=1).sum() >= min_length]


def drop_overlaps(segments, reference, tol=0.15):
    """
    Removes segments whose midpoint lies on a reference segment, such as hidden lines under
    visible ones.
    @param {np.ndarray} segments - (s, 2, 2) candidates
    @param {np.ndarray} reference - (r, 2, 2) segments to test against
    @param {float} tol - Distance in mm that counts as overlapping
    @returns {np.ndarray} - The remaining candidates
    """
    if len(reference) == 0 or len(segments) == 0:
        return segments
    mid = segments.mean(1)
    a, b = reference[:, 0], reference[:, 1]
    ab = b - a
    denom = np.maximum((ab * ab).sum(1), 1e-12)
    keep = np.ones(len(mid), bool)
    for s in range(0, len(mid), 512):
        p = mid[s:s + 512, None, :]
        t = np.clip(((p - a) * ab).sum(-1) / denom, 0, 1)
        dist = np.linalg.norm(p - (a + t[..., None] * ab), axis=-1).min(1)
        keep[s:s + 512] = dist > tol
    return segments[keep]
