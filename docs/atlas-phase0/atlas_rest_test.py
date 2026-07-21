"""End-to-end test of the C++ /atlas/build + /atlas/fit REST endpoints.

Builds an atlas from most of the corpus (with a nominal 'tissue' study-variable column so
fit can vote on it), then fits a handful of genuinely held-out datasets and reports the
neighbours, predicted tissue, confidence, OOD flag, and frame coverage.
"""
import json, os, collections
import requests

HERE = os.path.dirname(__file__)
BASE = os.environ.get("BASE", "http://localhost:28101/lipidspace/v1")
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
VOCAB = sorted(l for l, c in lip_studies.items() if len(c) >= 5)
VSET = set(VOCAB)
qrow = lambda f: ",".join('"' + str(x).replace('"', '""') + '"' for x in f)

usable = [d for d in sorted(corpus) if len(set(corpus[d]["lipids"]) & VSET) >= MINLIP]

# Hold out a few datasets spanning tissues (genuinely absent from the atlas).
held = []
seen = set()
for d in usable:
    t = htiss(corpus[d]["tissue"])
    if t not in seen:
        held.append(d); seen.add(t)
    if len(held) >= 8:
        break
build_ids = [d for d in usable if d not in held]


def build_table(ds_ids):
    rows = [qrow(["Sample", "tissue"] + VOCAB)]
    for d in ds_ids:
        lp = corpus[d]["lipids"]
        rows.append(qrow([d, htiss(corpus[d]["tissue"])] + [lp.get(l, "") for l in VOCAB]))
    return "\n".join(rows), ["SampleColumn", "StudyVariableColumnNominal"] + ["LipidColumn"] * len(VOCAB)


def query_table(d):
    lp = corpus[d]["lipids"]
    rows = [qrow(["Sample"] + VOCAB), qrow([d] + [lp.get(l, "") for l in VOCAB])]
    return "\n".join(rows), ["SampleColumn"] + ["LipidColumn"] * len(VOCAB)


print(f"building atlas from {len(build_ids)} datasets, holding out {len(held)}...")
tbl, cols = build_table(build_ids)
r = requests.post(f"{BASE}/atlas/build",
                  json={"TableType": "COLUMN_PIVOT_TABLE", "TableColumnTypes": cols,
                        "Table": tbl, "Modules": 20, "LabelVariable": "tissue"}, timeout=1800)
r.raise_for_status()
atlas = r.json()
print(f"  frame lipids : {len(atlas['frame'])}")
print(f"  modules K    : {atlas['K']}   dims: {atlas['dims']}   bandwidth: {atlas['bandwidth']:.4f}")
print(f"  datasets N   : {len(atlas['datasets'])}")
print(f"  ood_threshold: {atlas['ood_threshold']:.4f}   label: {atlas['label_variable']}")
print(f"  atlas JSON   : {len(json.dumps(atlas))/1e6:.2f} MB")
print(f"  transform    : ref_names={len(atlas.get('ref_names', []))}  "
      f"eigenvectors={len(atlas.get('eigenvectors', []))}x"
      f"{len(atlas.get('eigenvectors', [[]])[0]) if atlas.get('eigenvectors') else 0}")
print(f"  ROUND-TRIP   : max |reprojected - stored| coord = {atlas.get('roundtrip_error', 'n/a')}\n")

print("=== fit held-out queries ===")
correct = 0
for d in held:
    tbl, cols = query_table(d)
    r = requests.post(f"{BASE}/atlas/fit",
                      json={"Atlas": atlas, "TableType": "COLUMN_PIVOT_TABLE",
                            "TableColumnTypes": cols, "Table": tbl, "NumNeighbors": 5}, timeout=600)
    r.raise_for_status()
    res = r.json()["results"][0]
    truth = htiss(corpus[d]["tissue"])
    if "error" in res:
        print(f"\n{d} (true {truth}): {res['error']} (coverage {res.get('coverage')})")
        continue
    pred = res.get("prediction", "?")
    correct += (pred == truth)
    print(f"\n{d}  true={truth}  pred={pred}  conf={res['confidence']:.3f}  "
          f"ood={res['ood']}  cov={res['coverage']:.2f}  nn={res['nn_distance']:.3f}")
    for nb in res["neighbors"]:
        print(f"    {nb['dataset']:14s} d={nb['distance']:.3f}  {nb['metadata'].get('tissue','?')}")
print(f"\nheld-out tissue predictions correct: {correct}/{len(held)}")
