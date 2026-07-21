"""Build the Atlas artifact once: freeze the frame + modules, fingerprint every dataset,
calibrate confidence, and persist atlas.json."""
import json, os, collections, time
import requests
from atlas import Atlas

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
VOCAB = sorted(l for l, c in lip_studies.items() if len(c) >= 5)   # settled config
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
    return glob, samples


t0 = time.time()
glob, samples = fetch()
frame = {nm: [x, y] for nm, x, y in zip(glob["LipidNames"], glob["X"], glob["Y"])}
dataset_lipidomes, meta = {}, {}
for s in samples:
    ds = clean(s["LipidomeName"])
    dataset_lipidomes[ds] = {nm: w for nm, w in zip(s["LipidNames"], s["Intensities"]) if w > 0}
    meta[ds] = {"tissue": htiss(corpus[ds]["tissue"]),
                "species": corpus[ds]["species"], "study": study_of(ds)}

atlas = Atlas.build(frame, dataset_lipidomes, meta, K=20)
path = os.path.join(HERE, "atlas.json")
atlas.save(path)
json.dump(dataset_lipidomes, open(os.path.join(HERE, "demo_queries.json"), "w"))
dt = time.time() - t0
size_mb = os.path.getsize(path) / 1e6
print(f"built atlas in {dt:.1f}s")
print(f"  frozen frame lipids : {len(atlas.frame)}")
print(f"  modules (K)         : {len(atlas.centers)}")
print(f"  datasets (N)        : {len(atlas.datasets)}")
print(f"  fingerprint dim     : {atlas.F.shape[1]}")
print(f"  atlas.json size     : {size_mb:.2f} MB")
print(f"  calibration nn_ref  : median {sorted(atlas.nn_ref)[len(atlas.nn_ref)//2]:.3f}, "
      f"95th pct (OOD thr) {atlas.nn_ref[int(0.95*len(atlas.nn_ref))]:.3f}")
