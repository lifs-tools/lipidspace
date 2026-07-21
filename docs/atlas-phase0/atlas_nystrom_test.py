"""Test Nystrom projection at /atlas/fit.

Build the atlas over the shared>=5 vocabulary (the frozen frame). Then fit held-out datasets
two ways: (a) with vocab-only lipids (all in the frame -> lookup, no projection), and (b) with
their FULL lipid list (many lipids outside the frame -> Nystrom-projected). Projection should
place the out-of-frame lipids (n_projected > 0, coverage rises) and keep predictions sensible.
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

held = [d for d in usable if htiss(corpus[d]["tissue"]) in ("plasma", "brain", "liver")][:4]
build_ids = [d for d in usable if d not in held]


def build_table(ds_ids):
    rows = [qrow(["Sample", "tissue"] + VOCAB)]
    for d in ds_ids:
        lp = corpus[d]["lipids"]
        rows.append(qrow([d, htiss(corpus[d]["tissue"])] + [lp.get(l, "") for l in VOCAB]))
    return "\n".join(rows), ["SampleColumn", "StudyVariableColumnNominal"] + ["LipidColumn"] * len(VOCAB)


def query_table(d, lipids):
    lipids = sorted(lipids)
    lp = corpus[d]["lipids"]
    rows = [qrow(["Sample"] + lipids), qrow([d] + [lp.get(l, "") for l in lipids])]
    return "\n".join(rows), ["SampleColumn"] + ["LipidColumn"] * len(lipids)


print(f"building atlas from {len(build_ids)} datasets (frame = shared>=5 vocab)...")
tbl, cols = build_table(build_ids)
atlas = requests.post(f"{BASE}/atlas/build",
                      json={"TableType": "COLUMN_PIVOT_TABLE", "TableColumnTypes": cols,
                            "Table": tbl, "Modules": 20, "LabelVariable": "tissue"}, timeout=1800).json()
print(f"  frame lipids={len(atlas['frame'])}  ref_names={len(atlas['ref_names'])}  "
      f"roundtrip={atlas['roundtrip_error']:.2e}\n")


def fit(d, lipids):
    tbl, cols = query_table(d, lipids)
    return requests.post(f"{BASE}/atlas/fit",
                         json={"Atlas": atlas, "TableType": "COLUMN_PIVOT_TABLE",
                               "TableColumnTypes": cols, "Table": tbl, "NumNeighbors": 5},
                         timeout=1200).json()["results"][0]


for d in held:
    truth = htiss(corpus[d]["tissue"])
    full = list(corpus[d]["lipids"])
    invocab = [l for l in full if l in VSET]
    a = fit(d, invocab)                       # all in frame -> lookup only
    b = fit(d, full)                          # full list -> out-of-frame lipids projected
    print(f"{d}  true={truth}   (in-vocab {len(invocab)}, full {len(full)} lipids)")
    print(f"  vocab-only : pred={a.get('prediction')}  conf={a['confidence']:.3f}  "
          f"cov={a['coverage']:.2f}  projected={a.get('projected_lipids', 0)}  nn={a['nn_distance']:.3f}")
    print(f"  + Nystrom  : pred={b.get('prediction')}  conf={b['confidence']:.3f}  "
          f"cov={b['coverage']:.2f}  projected={b.get('projected_lipids', 0)}  nn={b['nn_distance']:.3f}")
