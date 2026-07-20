"""Validate the C++ fingerprint port behaviorally.

Fetch LipidSpace's OWN lipidome distance matrix for each metric via REST
(DistanceMetric = hellinger | hausdorff), run the same LODO metrics used in
evaluate.py, and compare against the Python fingerprint numbers. Exact distances
won't match Python (different k-means RNG/impl), but the retrieval metrics should.
"""
import json, os, collections
import numpy as np
import requests
from sklearn.metrics import balanced_accuracy_score

HERE = os.path.dirname(__file__)
URL = os.environ.get("URL", "http://localhost:28101/lipidspace/v1/pca")
MINFREQ = int(os.environ.get("MINFREQ", "3"))
MINLIP = int(os.environ.get("MINLIP", "10"))

corpus = json.load(open(os.path.join(HERE, "corpus.json")))
TISSUE = {"blood plasma": "plasma", "plasma": "plasma", "plasma/serum": "plasma",
          "serum": "serum", "blood serum": "serum",
          "brain": "brain", "different brain regions": "brain", "hippocampus": "brain"}


def htiss(t):
    return TISSUE.get(t, t)


def qrow(fields):
    return ",".join('"' + str(f).replace('"', '""') + '"' for f in fields)


def build_payload(ds_ids, lipids, metric):
    rows = [qrow(["Sample"] + lipids)]
    for ds in ds_ids:
        lp = corpus[ds]["lipids"]
        rows.append(qrow([ds] + [lp.get(l, "") for l in lipids]))
    return {"TableType": "COLUMN_PIVOT_TABLE",
            "TableColumnTypes": ["SampleColumn"] + ["LipidColumn"] * len(lipids),
            "Table": "\n".join(rows), "DistanceMetric": metric}


def is_sample(name):
    return name != "global_lipidome" and "group lipidome" not in name


def clean(name):
    suf = " - table_file"
    return name[:-len(suf)] if name.endswith(suf) else name


def fetch(metric):
    ds_ids = sorted(corpus)
    freq = collections.Counter()
    for ds in ds_ids:
        for l in corpus[ds]["lipids"]:
            freq[l] += 1
    lipids = sorted(l for l, c in freq.items() if c >= MINFREQ)
    lipset = set(lipids)
    ds_ids = [d for d in ds_ids if len(set(corpus[d]["lipids"]) & lipset) >= MINLIP]
    samples, H = None, None
    for _ in range(6):
        r = requests.post(URL, json=build_payload(ds_ids, lipids, metric), timeout=1800)
        if not r.ok:
            raise RuntimeError(f"HTTP {r.status_code} {r.reason} :: {r.text[:200]}")
        resp = r.json()
        spaces = resp["LipidSpaces"]
        samples = [s for s in spaces if is_sample(s["LipidomeName"])]
        H = resp["LipidomeDistanceMatrix"]
        if len(samples) == len(H):
            break
        k = len(samples) - len(H)
        idx = sorted(range(len(samples)), key=lambda i: len(samples[i]["LipidNames"]))[:k]
        drop = {clean(samples[i]["LipidomeName"]) for i in idx}
        ds_ids = [d for d in ds_ids if d not in drop]
    return [clean(s["LipidomeName"]) for s in samples], np.array(H, float)


def precision_at_k(D, labels, k=5):
    n = len(labels)
    return float(np.mean([np.mean(labels[np.argsort(D[i])[1:k + 1]] == labels[i])
                          for i in range(n)]))


def populated_lodo(D, labels, minsup=7, k=5):
    n = len(labels); cnt = collections.Counter(labels)
    idx = np.array([i for i in range(n) if cnt[labels[i]] >= minsup])
    lab, Dsub = labels[idx], D[np.ix_(idx, idx)]
    pred = [collections.Counter(lab[np.argsort(Dsub[a])[1:k + 1]]).most_common(1)[0][0]
            for a in range(len(idx))]
    return balanced_accuracy_score(lab, np.array(pred))


def pair_sep(D, labels, a, b, k=5):
    idx = np.where(np.isin(labels, [a, b]))[0]
    lab, Dsub = labels[idx], D[np.ix_(idx, idx)]
    pred = [collections.Counter(lab[np.argsort(Dsub[x])[1:k + 1]]).most_common(1)[0][0]
            for x in range(len(idx))]
    return balanced_accuracy_score(lab, np.array(pred))


for metric in ["hellinger", "hausdorff"]:
    print(f"\n=== C++ {metric} ===")
    try:
        order, D = fetch(metric)
        tissue = np.array([htiss(corpus[d]["tissue"]) for d in order])
        species = np.array([corpus[d]["species"] for d in order])
        finite = np.isfinite(D).all()
        print(f"  n={len(order)}  finite={finite}  symmetric={np.allclose(D, D.T)}  "
              f"diag0={np.allclose(np.diag(D), 0)}  range=[{np.nanmin(D):.3f},{np.nanmax(D):.3f}]")
        print(f"  precision@5  tissue={precision_at_k(D, tissue):.3f}  "
              f"species={precision_at_k(D, species):.3f}")
        print(f"  populated-tissue bal-acc={populated_lodo(D, tissue):.3f}")
        print(f"  plasma/brain={pair_sep(D, tissue, 'plasma', 'brain'):.3f}  "
              f"plasma/liver={pair_sep(D, tissue, 'plasma', 'liver'):.3f}")
    except Exception as e:
        print("  FAILED:", repr(e))

print("\n=== Python reference (evaluate.py) ===")
print("  fingerprint (K=20 soft): tissue p@5 ~0.35, plasma/brain ~0.87")
print("  Hausdorff ceiling      : tissue p@5  0.24, plasma/brain  0.71")
