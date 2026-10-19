"""Loads triangle meshes from 3MF files and derives edge topology."""

import zipfile
import xml.etree.ElementTree as ET
from dataclasses import dataclass, field

import numpy as np

NS = "{http://schemas.microsoft.com/3dmanufacturing/core/2015/02}"


@dataclass
class Mesh:
    name: str
    vertices: np.ndarray  # (n, 3) float, mm
    faces: np.ndarray  # (m, 3) int
    normals: np.ndarray = field(init=False)
    edges: np.ndarray = field(init=False)  # (k, 2) vertex indices
    edge_faces: np.ndarray = field(init=False)  # (k, 2) face indices, -1 on open edges

    def __post_init__(self):
        self.normals = face_normals(self.vertices, self.faces)
        self.edges, self.edge_faces = edge_topology(self.faces)


def load_3mf(path):
    """
    Reads all mesh objects of a 3MF file, merging duplicate vertices.
    @param {str} path - Path to the .3mf archive
    @returns {dict} - Object name mapped to Mesh
    """
    with zipfile.ZipFile(path) as archive:
        root = ET.fromstring(archive.read("3D/3dmodel.model"))
    meshes = {}
    for obj in root.iter(f"{NS}object"):
        mesh = obj.find(f"{NS}mesh")
        if mesh is None:
            continue
        verts = np.array(
            [[float(v.get(a)) for a in "xyz"] for v in mesh.iter(f"{NS}vertex")]
        )
        tris = np.array(
            [[int(t.get(a)) for a in ("v1", "v2", "v3")] for t in mesh.iter(f"{NS}triangle")]
        )
        verts, tris = merge_vertices(verts, tris)
        meshes[obj.get("name")] = Mesh(obj.get("name"), verts, tris)
    return meshes


def merge_vertices(verts, tris, tol=1e-4):
    """
    Collapses vertices closer than the tolerance and drops degenerate triangles.
    @param {np.ndarray} verts - Vertex positions
    @param {np.ndarray} tris - Triangle vertex indices
    @param {float} tol - Merge grid size in mm
    @returns {tuple} - Merged vertices and remapped triangles
    """
    keys = np.round(verts / tol).astype(np.int64)
    _, first, inverse = np.unique(keys, axis=0, return_index=True, return_inverse=True)
    tris = inverse.reshape(-1)[tris]
    ok = (tris[:, 0] != tris[:, 1]) & (tris[:, 1] != tris[:, 2]) & (tris[:, 0] != tris[:, 2])
    return verts[first], tris[ok]


def face_normals(verts, tris):
    """
    Computes unit normals per triangle.
    @param {np.ndarray} verts - Vertex positions
    @param {np.ndarray} tris - Triangle vertex indices
    @returns {np.ndarray} - (m, 3) unit normals
    """
    a, b, c = (verts[tris[:, i]] for i in range(3))
    n = np.cross(b - a, c - a)
    length = np.linalg.norm(n, axis=1, keepdims=True)
    return n / np.maximum(length, 1e-12)


def edge_topology(tris):
    """
    Lists the unique undirected edges and the (up to two) faces sharing each.
    @param {np.ndarray} tris - Triangle vertex indices
    @returns {tuple} - Edge vertex pairs and their adjacent face pairs
    """
    half = np.concatenate([tris[:, [0, 1]], tris[:, [1, 2]], tris[:, [2, 0]]])
    face_ids = np.tile(np.arange(len(tris)), 3)
    key = np.sort(half, axis=1)
    order = np.lexsort((key[:, 1], key[:, 0]))
    key, face_ids = key[order], face_ids[order]
    new = np.ones(len(key), bool)
    new[1:] = np.any(key[1:] != key[:-1], axis=1)
    starts = np.flatnonzero(new)
    edges = key[starts]
    pairs = np.full((len(edges), 2), -1)
    pairs[:, 0] = face_ids[starts]
    second = np.zeros(len(key), bool)
    second[1:] = ~new[1:]
    group = np.cumsum(new) - 1
    pairs[group[second], 1] = face_ids[second]
    return edges, pairs
