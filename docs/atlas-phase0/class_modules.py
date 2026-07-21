"""Class-stratified modules vs flat k-means.

Flat k-means clusters the whole frame at once; class-stratified partitions the frame by
lipid class and allocates modules per class (proportional to sqrt of class size, a proxy
for structural richness), so a lipid only contributes to modules of its own class. Both are
built in Python from the SAME LipidSpace frame coords, so the only difference is the module
structure. Compared on tissue recovery, batch dominance, and confidence AUROC.
"""
import json, os, collections
import numpy as np
import requests
from sklearn.cluster import KMeans
from sklearn.metrics import roc_auc_score
from scipy.spatial.distance import squareform, pdist

HERE = os.path.dirname(__file__)
URL = os.environ.get("URL", "http://localhost:28101/lipidspace/v1/pca")
MINLIP = 10
corpus = json.load(open(os.path.join(HERE, "corpus.json")))
TISSUE = {"blood plasma": "plasma", "plasma": "plasma", "plasma/serum": "plasma",
          "serum": "serum", "blood serum": "serum",
          "brain": "brain", "different brain regions": "brain", "hippocampus": "brain"}
htiss = lambda t: TISSUE.get(t, t)
study_of = lambda ds: "-".join(ds.split("-")[:2])
lipclass = lambda name: name.split()[0] if name.split() else name

lip_studies = collections.defaultdict(set)
for d in corpus:
    for l in corpus[d]["lipids"]:
        lip_studies[l].add(study_of(d))
VOCAB = sorted(l for l, c in lip_studies.items() if len(c) >= 5)
VSET = set(VOCAB)


def qrow(f):
    return ",".join('"' + str(x).replace('"', '""') + '"' for x in f)


def clean(name):
    s = " - table_file"
    return name[:-len(s)] if name.endswith(s) else name


def fetch():
    ds_ids = [d for d in sorted(corpus) if len(set(corpus[d]["lipids"]) & VSET) >= MINLIP]
    samples = H = glob = None
    for _ in range(8):
        rows = [qrow(["Sample"] + VOCAB)]
        for d in ds_ids:
            lp = corpus[d]["lipids"]
            rows.append(qrow([d] + [lp.get(l, "") for l in VOCAB]))
        r = requests.post(URL, json={"TableType": "COLUMN_PIVOT_TABLE",
                                     "TableColumnTypes": ["SampleColumn"] + ["LipidColumn"] * len(VOCAB),
                                     "Table": "\n".join(rows), "DistanceMetric": "hellinger"}, timeout=1800)
        r.raise_for_status(); resp = r.json()
        glob = next(s for s in resp["LipidSpaces"] if s["LipidomeName"] == "global_lipidome")
        samples = [s for s in resp["LipidSpaces"]
                   if s["LipidomeName"] != "global_lipidome" and "group lipidome" not in s["LipidomeName"]]
        H = resp["LipidomeDistanceMatrix"]
        if len(samples) == len(H):
            break
        kk = len(samples) - len(H)
        idx = sorted(range(len(samples)), key=lambda i: len(samples[i]["LipidNames"]))[:kk]
        drop = {clean(samples[i]["LipidomeName"]) for i in idx}
        ds_ids = [d for d in ds_ids if d not in drop]
    order = [clean(s["LipidomeName"]) for s in samples]
    frame_xy = np.array([[x, y] for x, y in zip(glob["X"], glob["Y"])], float)
    frame_cls = np.array([lipclass(nm) for nm in glob["LipidNames"]])
    clouds = {clean(s["LipidomeName"]):
              [(lipclass(nm), x, y, w) for nm, x, y, w
               in zip(s["LipidNames"], s["X"], s["Y"], s["Intensities"])]
              for s in samples}
    return order, np.array(H, float), frame_xy, frame_cls, clouds


order, Hcpp, frame_xy, frame_cls, clouds = fetch()
tissue = np.array([htiss(corpus[d]["tissue"]) for d in order])
studies = np.array([study_of(d) for d in order])
n = len(order)


def flat_modules(K):
    c = KMeans(K, n_init=4, random_state=0).fit(frame_xy).cluster_centers_
    return c, np.full(len(c), "_ALL_")


def strat_modules(K_total):
    counts = collections.Counter(frame_cls)
    w = {c: cnt ** 0.5 for c, cnt in counts.items()}; S = sum(w.values())
    centers, tags = [], []
    for c in sorted(counts):
        kc = min(counts[c], max(1, round(K_total * w[c] / S)))
        pts = frame_xy[frame_cls == c]
        km = KMeans(kc, n_init=2, random_state=0).fit(pts).cluster_centers_
        centers += list(km); tags += [c] * len(km)
    return np.array(centers), np.array(tags)


def fingerprints(centers, tags, stratified):
    by_class = {}
    for c in set(tags):
        by_class[c] = np.where(tags == c)[0]
    # bandwidth: median nearest-allowed-centroid distance over frame points
    md = []
    for i in range(len(frame_xy)):
        cand = by_class.get(frame_cls[i], np.arange(len(centers))) if stratified else np.arange(len(centers))
        if len(cand):
            md.append(np.linalg.norm(centers[cand] - frame_xy[i], axis=1).min())
    s = np.median(md) + 1e-9
    F = np.zeros((n, len(centers)))
    for idx, d in enumerate(order):
        for (c, x, y, w) in clouds[d]:
            cand = by_class.get(c) if stratified else np.arange(len(centers))
            if cand is None or len(cand) == 0:
                continue
            dc = np.linalg.norm(centers[cand] - np.array([x, y]), axis=1)
            a = np.exp(-(dc ** 2) / (2 * s * s)); a = a / (a.sum() + 1e-12)
            F[idx, cand] += w * a
    F = F / (F.sum(1, keepdims=True) + 1e-12)
    return squareform(pdist(np.sqrt(F), "euclidean")) / np.sqrt(2)


def prec(D, labels, k=5, cross=False):
    out = []
    for i in range(n):
        o = np.argsort(D[i]); o = o[o != i]
        if cross:
            o = o[studies[o] != studies[i]]
        o = o[:k]
        if len(o):
            out.append(np.mean(labels[o] == labels[i]))
    return float(np.mean(out))


def wb(D, labels):
    iu = np.triu_indices(n, 1); same = labels[iu[0]] == labels[iu[1]]; d = D[iu]
    return d[same].mean() / d[~same].mean()


def conf_auroc(D, k=5):
    correct, closeness = [], []
    for i in range(n):
        o = np.argsort(D[i]); o = o[o != i][:k]
        pred = collections.Counter(tissue[o]).most_common(1)[0][0]
        correct.append(pred == tissue[i]); closeness.append(-D[i, o[0]])
    return roc_auc_score(correct, closeness), float(np.mean(correct))


print(f"n={n}  frame lipids={len(frame_xy)}  classes={len(set(frame_cls))}\n")
print(f"{'modules':>22} {'#mod':>5} | {'tis_all':>7} {'tis_x':>6} {'wb_study':>8} | {'acc':>5} {'conf_auroc':>10}")
Dn = Hcpp / Hcpp.max()
a, ac = conf_auroc(Dn)
print(f"{'C++ flat K=20 (ref)':>22} {20:>5} | {prec(Dn, tissue):>7.3f} {prec(Dn, tissue, cross=True):>6.3f} "
      f"{wb(Dn, studies):>8.3f} | {ac:>5.3f} {a:>10.3f}")
for label, (ctr, tg) in [("py flat K=20", flat_modules(20)),
                         ("py flat K=50", flat_modules(50)),
                         ("class-strat ~50", strat_modules(50))]:
    D = fingerprints(ctr, tg, stratified=(tg[0] != "_ALL_"))
    a, ac = conf_auroc(D)
    print(f"{label:>22} {len(ctr):>5} | {prec(D, tissue):>7.3f} {prec(D, tissue, cross=True):>6.3f} "
          f"{wb(D, studies):>8.3f} | {ac:>5.3f} {a:>10.3f}")
