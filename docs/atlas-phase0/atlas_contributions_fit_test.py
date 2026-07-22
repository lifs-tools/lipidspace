"""Fits a query against a labeled atlas and asserts the contributions block:
dominant_lipids, per-variable attribution (lipids ranked, shares sum ~1,
modules with exemplars), top-N params honored, and a perturbation sanity check."""
import json, math, os, sys
import requests

BASE = os.environ.get("BASE", "http://localhost:28102/lipidspace/v1")
qf = lambda f: ('"' + str(f).replace('"', '""') + '"') if f != "" else ""

data = {
    "dsA": ("plasma", "Homo sapiens", {"PC 34:1": 120, "PC 36:2": 80, "SM 34:1;O2": 40, "PE 34:1": 15}),
    "dsB": ("brain",  "Homo sapiens", {"PC 34:1": 20,  "PC 36:2": 30, "SM 34:1;O2": 90, "PE 34:1": 60}),
    "dsC": ("plasma", "Mus musculus", {"PC 34:1": 110, "PC 36:2": 75, "SM 34:1;O2": 45, "PE 34:1": 18}),
    "dsD": ("brain",  "Mus musculus", {"PC 34:1": 25,  "PC 36:2": 28, "SM 34:1;O2": 85, "PE 34:1": 55}),
}
rows = [["Sample", "tissue", "species", "lipid", "quant"]]
for ds, (tis, sp, lipids) in data.items():
    for lip, q in lipids.items():
        rows.append([ds, tis, sp, lip, str(q)])
table = "\n".join(",".join(qf(c) for c in r) for r in rows)
atlas = requests.post(f"{BASE}/atlas/build", json={
    "TableType": "FLAT_TABLE",
    "TableColumnTypes": ["SampleColumn", "StudyVariableColumnNominal",
                         "StudyVariableColumnNominal", "LipidColumn", "QuantColumn"],
    "Table": table, "Modules": 5, "LabelVariable": "tissue"}, timeout=600).json()

def fit(query_lipids, **params):
    qrows = [["Sample"] + list(query_lipids.keys()), ["q"] + [str(v) for v in query_lipids.values()]]
    tbl = "\n".join(",".join(qf(c) for c in r) for r in qrows)
    body = {"Atlas": atlas, "TableType": "COLUMN_PIVOT_TABLE",
            "TableColumnTypes": ["SampleColumn"] + ["LipidColumn"] * len(query_lipids),
            "Table": tbl, "NumNeighbors": 3}
    body.update(params)
    return requests.post(f"{BASE}/atlas/fit", json=body, timeout=600).json()["results"][0]

# plasma-like query
q = {"PC 34:1": 115, "PC 36:2": 78, "SM 34:1;O2": 42, "PE 34:1": 16}
res = fit(q)
c = res.get("contributions", {})
dom = c.get("dominant_lipids", [])
byv = c.get("by_variable", {})
print("dominant_lipids:", [(d["lipid"], round(d["weight"], 3)) for d in dom])
print("predicted:", {k: res["predictions"][k]["prediction"] for k in res.get("predictions", {})})
for v in byv:
    print(f"  {v} -> {byv[v]['prediction']}: lipids=",
          [(l['lipid'], round(l['share'], 2)) for l in byv[v]['lipids']],
          " modules=", [(m['module'], len(m['exemplars'])) for m in byv[v]['modules']])

fail = []
if not dom: fail.append("no dominant_lipids")
if [d["weight"] for d in dom] != sorted((d["weight"] for d in dom), reverse=True):
    fail.append("dominant_lipids not sorted desc")
if set(byv.keys()) != set(res.get("predictions", {}).keys()):
    fail.append(f"by_variable keys {set(byv)} != predictions {set(res.get('predictions',{}))}")
for v, blk in byv.items():
    if blk["prediction"] != res["predictions"][v]["prediction"]:
        fail.append(f"{v} prediction mismatch")
    scores = [l["score"] for l in blk["lipids"]]
    if scores != sorted(scores, reverse=True): fail.append(f"{v} lipids not sorted desc")
    ssum = sum(l["share"] for l in blk["lipids"])
    if blk["lipids"] and abs(ssum - 1.0) > 0.02: fail.append(f"{v} shares sum {ssum} != 1")
    if any(len(m["exemplars"]) == 0 for m in blk["modules"]): fail.append(f"{v} a module has no exemplars")

# top-N params honored
res2 = fit(q, TopNDominantLipids=2, TopNLipids=1, TopNModules=2)
c2 = res2["contributions"]
if len(c2["dominant_lipids"]) != 2: fail.append("TopNDominantLipids not honored")
for v, blk in c2["by_variable"].items():
    if len(blk["lipids"]) != 1: fail.append(f"{v} TopNLipids not honored")
    if len(blk["modules"]) > 2: fail.append(f"{v} TopNModules not honored")
# 0 -> empty
res3 = fit(q, TopNDominantLipids=0)
if res3["contributions"]["dominant_lipids"] != []: fail.append("TopNDominantLipids=0 not empty")

# perturbation: dropping the top tissue-contributing lipid must not increase similarity
# to the predicted tissue (nn_distance to predicted class should not decrease).
tissue_lipids = byv.get("tissue", {}).get("lipids", [])
if tissue_lipids:
    top_lip = tissue_lipids[0]["lipid"]
    base_nn = res["nn_distance"]
    q_drop = {k: v for k, v in q.items() if k != top_lip}
    nn_drop = fit(q_drop)["nn_distance"]
    print(f"perturbation: drop {top_lip} -> nn {base_nn:.4f} -> {nn_drop:.4f}")
    if nn_drop < base_nn - 1e-9:
        fail.append(f"dropping top contributor DECREASED nn distance ({base_nn}->{nn_drop})")

if fail:
    print("FAIL:"); [print("  -", f) for f in fail]; sys.exit(1)
print("PASS: fit contributions correct")
