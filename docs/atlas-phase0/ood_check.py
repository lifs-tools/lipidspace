"""OOD / confidence scoring: can we flag the unreliable retrievals?

LODO tissue retrieval on the fingerprint (shared vocab). For each held-out dataset we
predict tissue by kNN majority and attach candidate confidence signals, then measure the
precision-coverage tradeoff (answer only the most confident X% -> does accuracy rise?) and
AUROC (does confidence rank correct predictions above wrong ones?). Also: do minority /
out-of-distribution tissues get flagged (lower confidence, farther nearest neighbour)?
"""
import json, os, collections
import numpy as np
import requests
from sklearn.metrics import roc_auc_score

HERE = os.path.dirname(__file__)
URL = os.environ.get("URL", "http://localhost:28101/lipidspace/v1/pca")
MINLIP = 10
K = 5
corpus = json.load(open(os.path.join(HERE, "corpus.json")))
TISSUE = {"blood plasma": "plasma", "plasma": "plasma", "plasma/serum": "plasma",
          "serum": "serum", "blood serum": "serum",
          "brain": "brain", "different brain regions": "brain", "hippocampus": "brain"}
htiss = lambda t: TISSUE.get(t, t)

lip_studies = collections.defaultdict(set)
for d in corpus:
    for l in corpus[d]["lipids"]:
        lip_studies[l].add("-".join(d.split("-")[:2]))
VOCAB = sorted(l for l, c in lip_studies.items() if len(c) >= 5)
VSET = set(VOCAB)


def qrow(f):
    return ",".join('"' + str(x).replace('"', '""') + '"' for x in f)


def clean(name):
    s = " - table_file"
    return name[:-len(s)] if name.endswith(s) else name


def fetch():
    ds_ids = [d for d in sorted(corpus) if len(set(corpus[d]["lipids"]) & VSET) >= MINLIP]
    samples = H = None
    for _ in range(8):
        rows = [qrow(["Sample"] + VOCAB)]
        for d in ds_ids:
            lp = corpus[d]["lipids"]
            rows.append(qrow([d] + [lp.get(l, "") for l in VOCAB]))
        r = requests.post(URL, json={"TableType": "COLUMN_PIVOT_TABLE",
                                     "TableColumnTypes": ["SampleColumn"] + ["LipidColumn"] * len(VOCAB),
                                     "Table": "\n".join(rows), "DistanceMetric": "hellinger"}, timeout=1800)
        r.raise_for_status(); resp = r.json()
        samples = [s for s in resp["LipidSpaces"]
                   if s["LipidomeName"] != "global_lipidome" and "group lipidome" not in s["LipidomeName"]]
        H = resp["LipidomeDistanceMatrix"]
        if len(samples) == len(H):
            break
        kk = len(samples) - len(H)
        idx = sorted(range(len(samples)), key=lambda i: len(samples[i]["LipidNames"]))[:kk]
        drop = {clean(samples[i]["LipidomeName"]) for i in idx}
        ds_ids = [d for d in ds_ids if d not in drop]
    return [clean(s["LipidomeName"]) for s in samples], np.array(H, float)


order, D = fetch()
tissue = np.array([htiss(corpus[d]["tissue"]) for d in order])
n = len(order)
sup = collections.Counter(tissue)

correct = np.zeros(n, bool)
conf = {s: np.zeros(n) for s in ("vote", "margin", "closeness", "density", "vote_x_close")}
nn_dist = np.zeros(n)
for i in range(n):
    o = np.argsort(D[i]); o = o[o != i]; top = o[:K]
    v = collections.Counter(tissue[top]).most_common()
    pred, c1 = v[0]; c2 = v[1][1] if len(v) > 1 else 0
    correct[i] = pred == tissue[i]
    nn_dist[i] = D[i, top[0]]
    conf["vote"][i] = c1 / K
    conf["margin"][i] = (c1 - c2) / K
    conf["closeness"][i] = -D[i, top[0]]
    conf["density"][i] = -D[i, top].mean()
    conf["vote_x_close"][i] = (c1 / K) * (-D[i, top[0]])

maj = sup.most_common(1)[0][1] / n
print(f"n={n}  overall kNN-majority tissue accuracy={correct.mean():.3f}  "
      f"(always-majority baseline={maj:.3f})\n")

covs = [1.0, 0.75, 0.50, 0.30, 0.15]
print(f"{'confidence':>13} {'AUROC':>6} |" + "".join(f"{int(c*100):>5}%" for c in covs)
      + "   (accuracy at coverage)")
for s, c in conf.items():
    idx = np.argsort(-c)
    row = "".join(f"{correct[idx[:max(1, round(cv*n))]].mean():>6.3f}" for cv in covs)
    print(f"{s:>13} {roc_auc_score(correct, c):>6.3f} |{row}")

# OOD proxy: minority (rare tissue) datasets should be flagged (lower confidence, farther NN)
minority = np.array([sup[t] < 7 for t in tissue])
print(f"\nOOD proxy (rare tissue, support<7: n={minority.sum()} vs populated n={(~minority).sum()}):")
print(f"  vote confidence   rare={conf['vote'][minority].mean():.3f}  populated={conf['vote'][~minority].mean():.3f}")
print(f"  nearest-nbr dist  rare={nn_dist[minority].mean():.3f}  populated={nn_dist[~minority].mean():.3f}  (higher=more OOD)")
