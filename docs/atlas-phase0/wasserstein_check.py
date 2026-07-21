"""Does structural Wasserstein reduce the batch confound, and does the fingerprint track it?

One /pca call gives the frozen frame coords + per-dataset point clouds (X, Y, Intensities)
and the fingerprint (Hellinger) matrix. From the clouds we compute:
  - centroid distance  (a provable Wasserstein lower bound; the cheap cascade filter)
  - sliced Wasserstein (structure-aware set distance; the expensive re-rank)
Then run the batch-vs-biology metrics on all three, and correlate fingerprint vs Wasserstein.
"""
import json, os, collections
import numpy as np
import requests
from scipy.spatial.distance import squareform, pdist
from scipy.stats import wasserstein_distance, spearmanr

HERE = os.path.dirname(__file__)
URL = os.environ.get("URL", "http://localhost:28101/lipidspace/v1/pca")
MINLIP = 10
corpus = json.load(open(os.path.join(HERE, "corpus.json")))
TISSUE = {"blood plasma": "plasma", "plasma": "plasma", "plasma/serum": "plasma",
          "serum": "serum", "blood serum": "serum",
          "brain": "brain", "different brain regions": "brain", "hippocampus": "brain"}
htiss = lambda t: TISSUE.get(t, t)
study_of = lambda ds: "-".join(ds.split("-")[:2])

lip_studies = collections.defaultdict(set)
for d in corpus:
    for l in corpus[d]["lipids"]:
        lip_studies[l].add(study_of(d))
VOCAB = sorted(l for l, c in lip_studies.items() if len(c) >= 5)   # shared>=5 (best config)
VSET = set(VOCAB)


def qrow(fields):
    return ",".join('"' + str(f).replace('"', '""') + '"' for f in fields)


def clean(name):
    suf = " - table_file"
    return name[:-len(suf)] if name.endswith(suf) else name


def fetch():
    ds_ids = [d for d in sorted(corpus) if len(set(corpus[d]["lipids"]) & VSET) >= MINLIP]
    samples = H = None
    for _ in range(8):
        rows = [qrow(["Sample"] + VOCAB)]
        for d in ds_ids:
            lp = corpus[d]["lipids"]
            rows.append(qrow([d] + [lp.get(l, "") for l in VOCAB]))
        payload = {"TableType": "COLUMN_PIVOT_TABLE",
                   "TableColumnTypes": ["SampleColumn"] + ["LipidColumn"] * len(VOCAB),
                   "Table": "\n".join(rows), "DistanceMetric": "hellinger"}
        r = requests.post(URL, json=payload, timeout=1800); r.raise_for_status()
        resp = r.json()
        samples = [s for s in resp["LipidSpaces"]
                   if s["LipidomeName"] != "global_lipidome" and "group lipidome" not in s["LipidomeName"]]
        H = resp["LipidomeDistanceMatrix"]
        if len(samples) == len(H):
            break
        k = len(samples) - len(H)
        idx = sorted(range(len(samples)), key=lambda i: len(samples[i]["LipidNames"]))[:k]
        drop = {clean(samples[i]["LipidomeName"]) for i in idx}
        ds_ids = [d for d in ds_ids if d not in drop]
    order = [clean(s["LipidomeName"]) for s in samples]
    clouds = {clean(s["LipidomeName"]):
              (np.array([[x, y] for x, y in zip(s["X"], s["Y"])], float),
               np.array(s["Intensities"], float))
              for s in samples}
    return order, np.array(H, float), clouds


def centroid_matrix(order, clouds):
    C = np.array([np.average(clouds[d][0], axis=0, weights=clouds[d][1]) for d in order])
    return squareform(pdist(C))


def sliced_wasserstein(order, clouds, L=25, seed=0):
    rng = np.random.default_rng(seed)
    dirs = rng.normal(size=(L, 2)); dirs /= np.linalg.norm(dirs, axis=1, keepdims=True)
    proj = {}
    for d in order:
        pts, w = clouds[d]; w = w / w.sum()
        proj[d] = (pts @ dirs.T, w)   # (npts, L)
    n = len(order); D = np.zeros((n, n))
    for i in range(n):
        pi, wi = proj[order[i]]
        for j in range(i + 1, n):
            pj, wj = proj[order[j]]
            s = sum(wasserstein_distance(pi[:, l], pj[:, l], wi, wj) for l in range(L)) / L
            D[i, j] = D[j, i] = s
    return D


def prec(D, labels, k=5, studies=None, cross=False):
    out = []
    for i in range(len(labels)):
        o = np.argsort(D[i]); o = o[o != i]
        if cross:
            o = o[studies[o] != studies[i]]
        o = o[:k]
        if len(o):
            out.append(np.mean(labels[o] == labels[i]))
    return float(np.mean(out))


def same_study(D, studies, k=5):
    return float(np.mean([np.mean(studies[np.argsort(D[i])[np.argsort(D[i]) != i][:k]] == studies[i])
                          for i in range(len(studies))]))


def wb(D, labels):
    iu = np.triu_indices(len(labels), 1); same = labels[iu[0]] == labels[iu[1]]; d = D[iu]
    return d[same].mean() / d[~same].mean()


order, Hfp, clouds = fetch()
tissue = np.array([htiss(corpus[d]["tissue"]) for d in order])
studies = np.array([study_of(d) for d in order])
Dc = centroid_matrix(order, clouds)
print("computing sliced Wasserstein ...")
Dw = sliced_wasserstein(order, clouds)

iu = np.triu_indices(len(order), 1)
print(f"\nrank corr (upper-tri): fingerprint~Wasserstein r={spearmanr(Hfp[iu], Dw[iu]).statistic:.3f}   "
      f"centroid~Wasserstein r={spearmanr(Dc[iu], Dw[iu]).statistic:.3f}")
print(f"\n{'distance':>14} | {'tis_all':>7} {'tis_x':>6} {'study':>5} {'ss_frac':>7} | {'wb_tis':>6} {'wb_study':>8}")
for name, D in [("fingerprint", Hfp), ("centroid(LB)", Dc), ("sliced-Wass", Dw)]:
    Dn = D / D.max()
    print(f"{name:>14} | {prec(Dn, tissue):>7.3f} {prec(Dn, tissue, studies=studies, cross=True):>6.3f} "
          f"{prec(Dn, studies):>5.3f} {same_study(Dn, studies):>7.3f} | "
          f"{wb(Dn, tissue):>6.3f} {wb(Dn, studies):>8.3f}")
