"""Asserts the atlas blob carries the build-time contribution data:
module_exemplars (K lists of frame-lipid names), class_baseline (K doubles),
class_signatures (per nominal variable/value mean fingerprint)."""
import json, os, sys
import requests

BASE = os.environ.get("BASE", "http://localhost:28102/lipidspace/v1")

rows = [["Sample", "tissue", "species", "lipid", "quant"]]
data = {
    "dsA": ("plasma", "Homo sapiens", {"PC 34:1": 120, "PC 36:2": 80, "SM 34:1;O2": 40, "PE 34:1": 15}),
    "dsB": ("brain",  "Homo sapiens", {"PC 34:1": 20,  "PC 36:2": 30, "SM 34:1;O2": 90, "PE 34:1": 60}),
    "dsC": ("plasma", "Mus musculus", {"PC 34:1": 110, "PC 36:2": 75, "SM 34:1;O2": 45, "PE 34:1": 18}),
}
for ds, (tis, sp, lipids) in data.items():
    for lip, q in lipids.items():
        rows.append([ds, tis, sp, lip, str(q)])
qf = lambda f: ('"' + f.replace('"', '""') + '"') if f != "" else ""
table = "\n".join(",".join(qf(c) for c in r) for r in rows)

req = {"TableType": "FLAT_TABLE",
       "TableColumnTypes": ["SampleColumn", "StudyVariableColumnNominal",
                            "StudyVariableColumnNominal", "LipidColumn", "QuantColumn"],
       "Table": table, "Modules": 5, "LabelVariable": "tissue"}
atlas = requests.post(f"{BASE}/atlas/build", json=req, timeout=600).json()

K = atlas["K"]
me = atlas.get("module_exemplars", [])
cb = atlas.get("class_baseline", [])
cs = atlas.get("class_signatures", {})
print("K:", K, "module_exemplars:", len(me), "class_baseline:", len(cb),
      "class_signatures vars:", sorted(cs.keys()))

fail = []
if len(me) != K: fail.append(f"module_exemplars length {len(me)} != K {K}")
if not all(isinstance(x, list) and len(x) >= 1 for x in me): fail.append("some module has no exemplars")
if len(cb) != K: fail.append(f"class_baseline length {len(cb)} != K {K}")
if abs(sum(cb) - 1.0) > 1e-6: fail.append(f"class_baseline should sum ~1, got {sum(cb)}")
if set(cs.keys()) != {"tissue", "species"}: fail.append(f"class_signatures vars {list(cs.keys())}")
if "plasma" not in cs.get("tissue", {}): fail.append("tissue/plasma signature missing")
if len(cs.get("tissue", {}).get("plasma", [])) != K: fail.append("plasma signature not length K")
if any(v == '"' or v == "" for v in cs.get("tissue", {})): fail.append("NA/empty leaked as a class")

if fail:
    print("FAIL:"); [print("  -", f) for f in fail]; sys.exit(1)
print("PASS: build-time contribution data present")
