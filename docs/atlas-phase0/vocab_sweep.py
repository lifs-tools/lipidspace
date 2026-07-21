"""Shared-vocabulary mitigation for batch confounding.

A lipid's *study* count = how many independent labs chose to report it. Restricting the
fingerprint to lipids many labs report (the shared / intersection vocabulary) should
strip out lab-specific panel-coverage signatures. We sweep the study-frequency threshold
and re-measure the batch-vs-biology metrics. If batch is a panel-coverage artefact, the
study within/between ratio should rise (batch less dominant) and cross-study tissue
recovery should climb.
"""
import json, os, collections
import numpy as np
import requests

HERE = os.path.dirname(__file__)
URL = os.environ.get("URL", "http://localhost:28101/lipidspace/v1/pca")
MINLIP = 10
corpus = json.load(open(os.path.join(HERE, "corpus.json")))
TISSUE = {"blood plasma": "plasma", "plasma": "plasma", "plasma/serum": "plasma",
          "serum": "serum", "blood serum": "serum",
          "brain": "brain", "different brain regions": "brain", "hippocampus": "brain"}
htiss = lambda t: TISSUE.get(t, t)
study_of = lambda ds: "-".join(ds.split("-")[:2])

# lipid -> number of distinct studies reporting it, and number of datasets
lip_studies = collections.defaultdict(set)
lip_datasets = collections.Counter()
for d in corpus:
    st = study_of(d)
    for l in corpus[d]["lipids"]:
        lip_studies[l].add(st)
        lip_datasets[l] += 1
lip_nstudies = {l: len(s) for l, s in lip_studies.items()}


def qrow(fields):
    return ",".join('"' + str(f).replace('"', '""') + '"' for f in fields)


def clean(name):
    suf = " - table_file"
    return name[:-len(suf)] if name.endswith(suf) else name


def fetch(lipids, metric="hellinger"):
    ds_ids = [d for d in sorted(corpus)
              if len(set(corpus[d]["lipids"]) & set(lipids)) >= MINLIP]
    samples = H = None
    for _ in range(8):
        rows = [qrow(["Sample"] + lipids)]
        for d in ds_ids:
            lp = corpus[d]["lipids"]
            rows.append(qrow([d] + [lp.get(l, "") for l in lipids]))
        payload = {"TableType": "COLUMN_PIVOT_TABLE",
                   "TableColumnTypes": ["SampleColumn"] + ["LipidColumn"] * len(lipids),
                   "Table": "\n".join(rows), "DistanceMetric": metric}
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


print("lipids by study threshold:",
      {n: sum(1 for c in lip_nstudies.values() if c >= n) for n in (2, 3, 5, 8, 12, 20)})
print(f"\n{'vocabulary':>22} {'#lip':>5} {'#ds':>4} | {'tis_all':>7} {'tis_x':>6} "
      f"{'study':>5} {'ss_frac':>7} | {'wb_tis':>6} {'wb_study':>8}")

configs = [("all (>=3 datasets)", sorted(l for l, c in lip_datasets.items() if c >= 3))]
configs += [(f">={n} studies", sorted(l for l, c in lip_nstudies.items() if c >= n))
            for n in (3, 5, 8, 12)]

for label, lipids in configs:
    if len(lipids) < 25:
        print(f"{label:>22} {len(lipids):>5}  (too few lipids, skipped)")
        continue
    order, D = fetch(lipids)
    D = D / D.max()
    tissue = np.array([htiss(corpus[d]["tissue"]) for d in order])
    studies = np.array([study_of(d) for d in order])
    print(f"{label:>22} {len(lipids):>5} {len(order):>4} | "
          f"{prec(D, tissue):>7.3f} {prec(D, tissue, studies=studies, cross=True):>6.3f} "
          f"{prec(D, studies):>5.3f} {same_study(D, studies):>7.3f} | "
          f"{wb(D, tissue):>6.3f} {wb(D, studies):>8.3f}")
