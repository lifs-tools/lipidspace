"""Build an Atlas with a broad SUPPORT frame (SwissLipids / LIPID MAPS species-level lipids).

The support lipids are sent as one frame-only pseudo-sample (FrameOnlySamples=["SUPPORT"]) in a
sparse FLAT table, so they define the PCA frame + modules but are not stored as datasets. The
corpus datasets (their shared>=5 vocab lipids) are fingerprinted against this broad frame.

SUPPORT env var selects the source: lipidmaps | swisslipids | union (default lipidmaps).
"""
import json, os, collections, time
import requests

HERE = os.path.dirname(__file__)
BASE = os.environ.get("BASE", "http://localhost:28101/lipidspace/v1")
SOURCE = os.environ.get("SUPPORT", "lipidmaps")
GOSLIN = "/Users/nilshoffmann/Projects/github.com/lifs-tools/goslin-webapp/src/main/resources"
SL = os.path.join(GOSLIN, "swiss-lipids-normalized.tsv")
LM = os.path.join(GOSLIN, "lipidmaps-normalized.tsv")
MINLIP = 10
corpus = json.load(open(os.path.join(HERE, "corpus.json")))
TISSUE = {"blood plasma": "plasma", "plasma": "plasma", "plasma/serum": "plasma",
          "serum": "serum", "blood serum": "serum",
          "brain": "brain", "different brain regions": "brain", "hippocampus": "brain"}
htiss = lambda t: TISSUE.get(t, t)
study_of = lambda ds: "-".join(ds.split("-")[:2])
qrow = lambda f: ",".join('"' + str(x).replace('"', '""') + '"' for x in f)


def species_names(path):
    names = set()
    with open(path) as f:
        next(f)
        for line in f:
            c = line.rstrip("\n").split("\t")
            if len(c) >= 6 and c[2] == "SPECIES" and c[5] and c[5] != "NA":
                names.add(c[5])
    return names


support = set()
if SOURCE in ("lipidmaps", "union"):
    support |= species_names(LM)
if SOURCE in ("swisslipids", "union"):
    support |= species_names(SL)
print(f"support source={SOURCE}: {len(support)} unique species-level reference lipids")

lip_studies = collections.defaultdict(set)
for d in corpus:
    for l in corpus[d]["lipids"]:
        lip_studies[l].add(study_of(d))
VOCAB = sorted(l for l, c in lip_studies.items() if len(c) >= 5)
VSET = set(VOCAB)
usable = [d for d in sorted(corpus) if len(set(corpus[d]["lipids"]) & VSET) >= MINLIP]
held = [d for d in usable if htiss(corpus[d]["tissue"]) in ("plasma", "brain", "liver")][:3]
build_ids = [d for d in usable if d not in held]

# FLAT table: Sample, tissue, lipid, quant
cols = ["SampleColumn", "StudyVariableColumnNominal", "LipidColumn", "QuantColumn"]
rows = [qrow(["Sample", "tissue", "lipid", "quant"])]
for d in build_ids:
    t = htiss(corpus[d]["tissue"])
    for l, q in corpus[d]["lipids"].items():
        if l in VSET and q > 0:
            rows.append(qrow([d, t, l, q]))
for s in sorted(support):
    rows.append(qrow(["SUPPORT", "support", s, 1]))
table_str = "\n".join(rows)
print(f"FLAT table: {len(rows)-1} rows ({len(build_ids)} corpus datasets + SUPPORT), "
      f"~{len(table_str)/1e6:.1f} MB")

print("building atlas (this can take minutes for large support sets)...")
t0 = time.time()
r = requests.post(f"{BASE}/atlas/build",
                  json={"TableType": "FLAT_TABLE", "TableColumnTypes": cols,
                        "Table": table_str, "Modules": 20, "LabelVariable": "tissue",
                        "FrameOnlySamples": ["SUPPORT"]}, timeout=7200)
r.raise_for_status()
atlas = r.json()
dt = time.time() - t0
print(f"built in {dt:.0f}s")
print(f"  frame lipids : {len(atlas['frame'])}   (corpus vocab + support, deduped)")
print(f"  datasets N   : {len(atlas['datasets'])}   (SUPPORT excluded: {'SUPPORT' not in atlas['datasets']})")
print(f"  modules K    : {atlas['K']}   dims: {atlas['dims']}")
print(f"  ROUND-TRIP   : {atlas['roundtrip_error']:.2e}")
print(f"  atlas JSON   : {len(json.dumps(atlas))/1e6:.1f} MB")
json.dump(atlas, open(os.path.join(HERE, f"atlas_support_{SOURCE}.json"), "w"))

# Fit held-out queries with their FULL lipid list; broad frame -> more direct lookups.
print("\n=== fit held-out queries (full lipid lists) ===")
for d in held:
    lipids = sorted(corpus[d]["lipids"])
    lp = corpus[d]["lipids"]
    qt = [qrow(["Sample"] + lipids), qrow([d] + [lp[l] for l in lipids])]
    res = requests.post(f"{BASE}/atlas/fit",
                        json={"Atlas": atlas, "TableType": "COLUMN_PIVOT_TABLE",
                              "TableColumnTypes": ["SampleColumn"] + ["LipidColumn"] * len(lipids),
                              "Table": "\n".join(qt), "NumNeighbors": 5}, timeout=1200).json()["results"][0]
    print(f"  {d}  true={htiss(corpus[d]['tissue'])}  pred={res.get('prediction')}  "
          f"conf={res['confidence']:.3f}  cov={res['coverage']:.2f}  "
          f"projected={res.get('projected_lipids', 0)}/{len(lipids)}  ood={res['ood']}")
