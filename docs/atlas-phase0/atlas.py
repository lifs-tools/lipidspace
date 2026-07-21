"""Reference implementation of the incremental Atlas.

A frozen structural frame (lipid -> coords), fixed k-means modules, and a persisted
per-dataset fingerprint store, with:
  - build once  : O(L^2) frame (done by LipidSpace) + O(N * size) fingerprints
  - fit a query : O(query_size + N)  (fingerprint + brute-force NN; O(log N) with ANN)
  - add dataset : O(size)            (fingerprint + append; no rebuild of anything)
plus a self-calibrated confidence / OOD flag from the atlas's own NN-distance distribution.

This is the architecture spec; the hot path (fit) ports to C++/LipidCompass, with the
brute-force NN replaced by an ANN index over the fingerprint vectors.
"""
import json
from collections import Counter
import numpy as np
from sklearn.cluster import KMeans
from scipy.spatial.distance import cdist


def build_modules(frame_coords, K, seed=0):
    km = KMeans(n_clusters=K, n_init=4, random_state=seed).fit(frame_coords)
    s = float(np.median(cdist(frame_coords, km.cluster_centers_).min(1)) + 1e-9)
    return km.cluster_centers_, s


def fingerprint(coords, weights, centers, s):
    coords = np.asarray(coords, float); weights = np.asarray(weights, float)
    A = np.exp(-(cdist(coords, centers) ** 2) / (2 * s * s))
    A /= A.sum(1, keepdims=True) + 1e-12
    f = (weights[:, None] * A).sum(0)
    return f / (f.sum() + 1e-12)


def fingerprint_lipidome(lipid_weights, frame, centers, s):
    """lipid_weights: {canonical_name: abundance}; coords come from the frozen frame."""
    coords, w = [], []
    for name, ab in lipid_weights.items():
        c = frame.get(name)
        if c is not None and ab > 0:
            coords.append(c); w.append(ab)
    cov = len(coords) / max(1, len(lipid_weights))
    if not coords:
        return None, cov
    return fingerprint(coords, w, centers, s), cov


def _hell(F, f):
    return np.linalg.norm(np.sqrt(F) - np.sqrt(f), axis=1) / np.sqrt(2)


class Atlas:
    def __init__(self, frame, centers, s, datasets, F, meta, nn_ref, config):
        self.frame = frame                        # {canonical_name: [x, y]}
        self.centers = np.asarray(centers, float)  # (K, d)
        self.s = s
        self.datasets = list(datasets)            # row order of F
        self.F = np.asarray(F, float)             # (N, K) fingerprints
        self.meta = meta                          # {ds: {tissue, species, study}}
        self.nn_ref = np.sort(np.asarray(nn_ref, float))
        self.config = config

    def fit(self, lipid_weights, k=5, exclude=None):
        f, cov = fingerprint_lipidome(lipid_weights, self.frame, self.centers, self.s)
        if f is None:
            return {"error": "no query lipids fell in the frozen frame", "coverage": cov}
        D = _hell(self.F, f)
        idx = np.argsort(D)
        if exclude is not None:
            idx = np.array([i for i in idx if self.datasets[i] != exclude])
        top = idx[:k]
        nn = float(D[top[0]])
        votes = [self.meta[self.datasets[i]]["tissue"] for i in top]
        pred, c1 = Counter(votes).most_common(1)[0]
        confidence = float(np.mean(self.nn_ref > nn))          # closer than this % of atlas
        ood_thr = float(np.quantile(self.nn_ref, 0.95))
        return {
            "neighbors": [(self.datasets[i], round(float(D[i]), 4),
                           self.meta[self.datasets[i]]["tissue"]) for i in top],
            "pred_tissue": pred, "vote": round(c1 / len(top), 3),
            "nn_dist": round(nn, 4), "confidence": round(confidence, 3),
            "ood": bool(nn > ood_thr), "coverage": round(cov, 3),
        }

    def add_dataset(self, ds_id, lipid_weights, meta):
        f, cov = fingerprint_lipidome(lipid_weights, self.frame, self.centers, self.s)
        if f is None:
            return False
        self.datasets.append(ds_id)
        self.F = np.vstack([self.F, f]) if self.F.size else f[None, :]
        self.meta[ds_id] = meta
        return True

    def save(self, path):
        json.dump({"frame": self.frame, "centers": self.centers.tolist(), "s": self.s,
                   "datasets": self.datasets, "F": self.F.tolist(), "meta": self.meta,
                   "nn_ref": self.nn_ref.tolist(), "config": self.config},
                  open(path, "w"))

    @classmethod
    def load(cls, path):
        d = json.load(open(path))
        return cls(d["frame"], d["centers"], d["s"], d["datasets"], d["F"],
                   d["meta"], d["nn_ref"], d["config"])

    @classmethod
    def build(cls, frame, dataset_lipidomes, meta, K=20, seed=0):
        """frame: {name:[x,y]}; dataset_lipidomes: {ds: {name: abundance}}."""
        fc = np.array(list(frame.values()), float)
        centers, s = build_modules(fc, K, seed)
        datasets, rows = [], []
        for ds, lw in dataset_lipidomes.items():
            f, _ = fingerprint_lipidome(lw, frame, centers, s)
            if f is not None:
                datasets.append(ds); rows.append(f)
        F = np.array(rows)
        # calibration: each atlas dataset's nearest-neighbour distance to another dataset
        nn_ref = []
        for i in range(len(datasets)):
            d = _hell(F, F[i]); d[i] = np.inf
            nn_ref.append(float(d.min()))
        return cls(frame, centers, s, datasets, F, meta,
                   nn_ref, {"K": K, "seed": seed})
