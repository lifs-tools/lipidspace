"""Batch-vs-biology check.

Each dataset is one study/lab/instrument (the platform/batch unit); tissues span many
studies. If the fingerprint recovers biology rather than batch, then (a) the same tissue
should cohere ACROSS studies (cross-study retrieval stays high), (b) a dataset's nearest
neighbours should not be dominated by its own study, and (c) tissue should group the
distances more tightly than study/instrument.
"""
import json, os, collections
import numpy as np
import requests

HERE = os.path.dirname(__file__)
URL = os.environ.get("URL", "http://localhost:28101/lipidspace/v1/pca")
MINFREQ, MINLIP = 3, 10
STUDIES_DIR = "/Users/nilshoffmann/Projects/github.com/lifs-tools/lipidcompass-studies/studies"

corpus = json.load(open(os.path.join(HERE, "corpus.json")))
TISSUE = {"blood plasma": "plasma", "plasma": "plasma", "plasma/serum": "plasma",
          "serum": "serum", "blood serum": "serum",
          "brain": "brain", "different brain regions": "brain", "hippocampus": "brain"}
htiss = lambda t: TISSUE.get(t, t)
study_of = lambda ds: "-".join(ds.split("-")[:2])

_instr = {}


def instrument_of(ds):
    if ds in _instr:
        return _instr[ds]
    path = os.path.join(STUDIES_DIR, study_of(ds), "data", ds + ".mzTab")
    name = "unknown"
    try:
        with open(path, encoding="utf-8", errors="replace") as f:
            for line in f:
                if line.startswith("MTD\tinstrument[1]-name"):
                    v = line.rstrip("\n").split("\t")[2]
                    parts = [p.strip().strip('"') for p in v.strip("[]").split(",")]
                    name = parts[2] if len(parts) >= 3 and parts[2] else "unknown"
                    break
                if line[:3] in ("SMH", "SML"):
                    break
    except Exception:
        pass
    _instr[ds] = name
    return name


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
    samples = H = None
    for _ in range(6):
        r = requests.post(URL, json=build_payload(ds_ids, lipids, metric), timeout=1800)
        r.raise_for_status()
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
    return [clean(s["LipidomeName"]) for s in samples], np.array(H, float)


def prec_at_k(D, labels, k=5, studies=None, cross_study=False):
    n = len(labels); out = []
    for i in range(n):
        order = np.argsort(D[i]); order = order[order != i]
        if cross_study:
            order = order[studies[order] != studies[i]]
        order = order[:k]
        if len(order):
            out.append(np.mean(labels[order] == labels[i]))
    return float(np.mean(out))


def same_study_frac(D, studies, k=5):
    n = len(studies)
    return float(np.mean([np.mean(studies[np.argsort(D[i])[np.argsort(D[i]) != i][:k]] == studies[i])
                          for i in range(n)]))


def within_between(D, labels):
    iu = np.triu_indices(len(labels), 1)
    same = labels[iu[0]] == labels[iu[1]]
    d = D[iu]
    return d[same].mean() / d[~same].mean()


for metric in ["hellinger", "hausdorff"]:
    order, D = fetch(metric)
    D = D / D.max()  # scale so within/between ratios are comparable across metrics
    tissue = np.array([htiss(corpus[d]["tissue"]) for d in order])
    species = np.array([corpus[d]["species"] for d in order])
    studies = np.array([study_of(d) for d in order])
    instr = np.array([instrument_of(d) for d in order])
    n = len(order)
    cnt = collections.Counter(studies)
    chance_ss = np.mean([(cnt[studies[i]] - 1) / (n - 1) for i in range(n)])

    print(f"\n=== {metric}  (n={n}) ===")
    print(f"  precision@5   tissue={prec_at_k(D, tissue):.3f}   "
          f"study={prec_at_k(D, studies):.3f}   instrument={prec_at_k(D, instr):.3f}")
    print(f"  tissue precision@5:  all-neighbours={prec_at_k(D, tissue):.3f}   "
          f"cross-study-only={prec_at_k(D, tissue, studies=studies, cross_study=True):.3f}")
    print(f"  same-study fraction in top-5={same_study_frac(D, studies):.3f}  (chance {chance_ss:.3f})")
    print(f"  within/between distance ratio  tissue={within_between(D, tissue):.3f}  "
          f"study={within_between(D, studies):.3f}  instrument={within_between(D, instr):.3f}   (lower=tighter)")
