"""Rank / presence fingerprints vs abundance, to target platform-quantification batch.

The residual batch effect is in abundance MAGNITUDES (labs calibrate differently). We
send transformed per-sample weights to LipidSpace (the frame/modules are structural and
unaffected; only the fingerprint weighting changes):
  - abundance : per-sample relative composition (baseline)
  - rank      : within-sample abundance rank, normalized (keeps order, drops magnitude)
  - presence  : 1 for every reported lipid (drops order and magnitude)
across the full vocabulary and the shared (>=5 studies) vocabulary. If magnitude is the
batch driver, rank should raise the within/between-study ratio and cross-study tissue.
"""
import json, os, collections
import numpy as np
import requests
from scipy.stats import rankdata

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
lip_datasets = collections.Counter()
for d in corpus:
    for l in corpus[d]["lipids"]:
        lip_studies[l].add(study_of(d))
        lip_datasets[l] += 1
VOCAB = {
    "full": sorted(l for l, c in lip_datasets.items() if c >= 3),
    "shared>=5": sorted(l for l, c in lip_studies.items() if len(c) >= 5),
}


def weights_for(d, vocab_set, scheme):
    present = {l: corpus[d]["lipids"][l] for l in corpus[d]["lipids"] if l in vocab_set}
    if scheme == "abundance":
        return present
    if scheme == "presence":
        return {l: 1.0 for l in present}
    ls = list(present)
    r = rankdata(np.array([present[l] for l in ls]))  # 1..n, higher abundance -> higher
    r = r / r.max()
    return {ls[i]: float(r[i]) for i in range(len(ls))}


def qrow(fields):
    return ",".join('"' + str(f).replace('"', '""') + '"' for f in fields)


def clean(name):
    suf = " - table_file"
    return name[:-len(suf)] if name.endswith(suf) else name


def fetch(vocab_name, scheme):
    vocab = VOCAB[vocab_name]; vset = set(vocab)
    ds_ids = [d for d in sorted(corpus)
              if len(set(corpus[d]["lipids"]) & vset) >= MINLIP]
    samples = H = None
    for _ in range(8):
        rows = [qrow(["Sample"] + vocab)]
        for d in ds_ids:
            w = weights_for(d, vset, scheme)
            rows.append(qrow([d] + [w.get(l, "") for l in vocab]))
        payload = {"TableType": "COLUMN_PIVOT_TABLE",
                   "TableColumnTypes": ["SampleColumn"] + ["LipidColumn"] * len(vocab),
                   "Table": "\n".join(rows), "DistanceMetric": "hellinger"}
        r = requests.post(URL, json=payload, timeout=1800)
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
    iu = np.triu_indices(len(labels), 1)
    same = labels[iu[0]] == labels[iu[1]]
    d = D[iu]
    return d[same].mean() / d[~same].mean()


print(f"{'weighting':>11} {'vocab':>10} {'#ds':>4} | {'tis_all':>7} {'tis_x':>6} "
      f"{'study':>5} {'ss_frac':>7} | {'wb_tis':>6} {'wb_study':>8}")
for scheme in ("abundance", "rank", "presence"):
    for vname in ("full", "shared>=5"):
        order, D = fetch(vname, scheme)
        D = D / D.max()
        tissue = np.array([htiss(corpus[d]["tissue"]) for d in order])
        studies = np.array([study_of(d) for d in order])
        print(f"{scheme:>11} {vname:>10} {len(order):>4} | "
              f"{prec(D, tissue):>7.3f} {prec(D, tissue, studies=studies, cross=True):>6.3f} "
              f"{prec(D, studies):>5.3f} {same_study(D, studies):>7.3f} | "
              f"{wb(D, tissue):>6.3f} {wb(D, studies):>8.3f}")
