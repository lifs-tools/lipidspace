"""Phase 0 — modules, fingerprints, and imbalance-robust LODO evaluation.

Compares the cheap fixed-length structural fingerprint against LipidSpace's own
Hausdorff matrix (the ceiling) on the real 179-dataset corpus. Aggregate multi-class
accuracy collapses to the majority under kNN on this biased corpus (true for the
Hausdorff ceiling too), so we use imbalance-robust reads:
  - retrieval precision@k: fraction of a dataset's k nearest that share its label
    (the actual "find similar datasets" behaviour), vs a random baseline
  - populated-class-only LODO balanced accuracy (classes with support >= 7)
  - pairwise strong contrasts (plasma/brain, plasma/liver, brain/liver)
  - Mantel (Spearman) agreement of fingerprint distances vs Hausdorff

Frame = LipidSpace 2D PCA (all the REST exposes); quantity (relative composition) is
folded into both Hausdorff and fingerprint. Read as a lower bound: more PCA dimensions
and class-stratified modules are the obvious levers.
"""
import os, json, warnings, collections
import numpy as np
from sklearn.cluster import KMeans
from sklearn.metrics import balanced_accuracy_score
from scipy.spatial.distance import squareform, pdist, cdist
from scipy.stats import spearmanr
warnings.filterwarnings("ignore")

HERE = os.path.dirname(__file__)
R = json.load(open(os.path.join(HERE, "lsresult.json")))  # self-generated JSON, no pickle

TISSUE = {"blood plasma": "plasma", "plasma": "plasma", "plasma/serum": "plasma",
          "serum": "serum", "blood serum": "serum",
          "brain": "brain", "different brain regions": "brain", "hippocampus": "brain"}

order = R["order"]
frame = np.array(R["frame_xy"], float)
H = np.array(R["hausdorff"], float)
clouds = {d: np.array(R["clouds"][d], float) for d in order}
tissue = np.array([TISSUE.get(R["meta"][d]["tissue"], R["meta"][d]["tissue"]) for d in order])
species = np.array([R["meta"][d]["species"] for d in order])
n = len(order)


def build_modules(K, seed=0):
    return KMeans(n_clusters=K, random_state=seed, n_init=4).fit(frame).cluster_centers_


def fingerprints(centers, soft=True, temper=1.0):
    d0 = cdist(frame, centers)
    s = temper * np.median(d0.min(1)) + 1e-9
    F = np.zeros((n, len(centers)))
    for i, d in enumerate(order):
        pts, w = clouds[d][:, :-1], clouds[d][:, -1]
        dc = cdist(pts, centers)
        if soft:
            A = np.exp(-(dc ** 2) / (2 * s * s)); A /= A.sum(1, keepdims=True) + 1e-12
        else:
            A = np.zeros_like(dc); A[np.arange(len(pts)), dc.argmin(1)] = 1.0
        F[i] = (w[:, None] * A).sum(0)
    return F / (F.sum(1, keepdims=True) + 1e-12)


def dist_matrix(F, metric):
    if metric == "cosine":
        return squareform(pdist(F, "cosine"))
    if metric == "braycurtis":
        return squareform(pdist(F, "braycurtis"))
    if metric == "jsd":
        return squareform(pdist(np.sqrt(F), "euclidean")) / np.sqrt(2)  # Hellinger
    raise ValueError(metric)


def precision_at_k(D, labels, k=5):
    p = [np.mean(labels[np.argsort(D[i])[1:k + 1]] == labels[i]) for i in range(n)]
    return float(np.mean(p))


def random_precision(labels):
    cnt = collections.Counter(labels)
    return float(np.mean([(cnt[labels[i]] - 1) / (n - 1) for i in range(n)]))


def populated_lodo(D, labels, minsup=7, k=5):
    cnt = collections.Counter(labels)
    idx = np.array([i for i in range(n) if cnt[labels[i]] >= minsup])
    lab, Dsub = labels[idx], D[np.ix_(idx, idx)]
    pred = [collections.Counter(lab[np.argsort(Dsub[a])[1:k + 1]]).most_common(1)[0][0]
            for a in range(len(idx))]
    return balanced_accuracy_score(lab, np.array(pred)), len(idx), sorted(set(lab.tolist()))


def pair_sep(D, a, b, k=5):
    idx = np.where(np.isin(tissue, [a, b]))[0]
    lab, Dsub = tissue[idx], D[np.ix_(idx, idx)]
    pred = [collections.Counter(lab[np.argsort(Dsub[x])[1:k + 1]]).most_common(1)[0][0]
            for x in range(len(idx))]
    return balanced_accuracy_score(lab, np.array(pred)), len(idx)


def mantel(D1, D2):
    iu = np.triu_indices(n, 1)
    return spearmanr(D1[iu], D2[iu]).statistic


# ---- corpus ----
print(f"datasets={n}")
print("tissue :", dict(collections.Counter(tissue).most_common(8)))
print(f"random precision@5: tissue={random_precision(tissue):.3f} species={random_precision(species):.3f}")

# ---- ceiling ----
pt = populated_lodo(H, tissue)
print("\nCEILING  (LipidSpace Hausdorff)")
print(f"  precision@5 tissue={precision_at_k(H, tissue):.3f}  species={precision_at_k(H, species):.3f}")
print(f"  populated-tissue LODO bal-acc={pt[0]:.3f} (n={pt[1]}, {pt[2]})")
print(f"  plasma/brain={pair_sep(H,'plasma','brain')[0]:.3f} "
      f"plasma/liver={pair_sep(H,'plasma','liver')[0]:.3f} "
      f"brain/liver={pair_sep(H,'brain','liver')[0]:.3f}")

# ---- fingerprint sweep ----
print("\nFINGERPRINT sweep")
print(f"{'K':>4} {'metric':>10} {'asg':>4} | {'p@5tis':>6} {'p@5sp':>6} {'popBacc':>7} "
      f"{'mantel':>6} | {'pl/br':>5} {'pl/li':>5} {'br/li':>5}")
best = None
for K in (20, 50, 100, 200):
    centers = build_modules(K)
    for soft in (False, True):
        F = fingerprints(centers, soft=soft)
        for metric in ("cosine", "jsd", "braycurtis"):
            D = dist_matrix(F, metric)
            pt5, ps5 = precision_at_k(D, tissue), precision_at_k(D, species)
            pb = populated_lodo(D, tissue)[0]
            mt = mantel(D, H)
            s1 = pair_sep(D, "plasma", "brain")[0]
            s2 = pair_sep(D, "plasma", "liver")[0]
            s3 = pair_sep(D, "brain", "liver")[0]
            print(f"{K:>4} {metric:>10} {'soft' if soft else 'hard':>4} | {pt5:>6.3f} "
                  f"{ps5:>6.3f} {pb:>7.3f} {mt:>6.3f} | {s1:>5.3f} {s2:>5.3f} {s3:>5.3f}")
            sc = pt5 + s1
            if best is None or sc > best[0]:
                best = (sc, K, metric, "soft" if soft else "hard", pt5, ps5, pb, mt, s1)

print(f"\nbest(p@5tis+pl/br): K={best[1]} {best[2]} {best[3]} -> "
      f"p@5tis={best[4]:.3f} p@5sp={best[5]:.3f} popBacc={best[6]:.3f} "
      f"mantel={best[7]:.3f} plasma/brain={best[8]:.3f}")
