"""Regression test: the FrameOnlySamples support sample must NOT leak into the atlas datasets,
dataset names must be the raw sample names (no ' - table_file' suffix), and the REST temp-file
'Origin' pseudo-variable must not pollute the per-dataset metadata / label variables.
"""
import json, os, sys
import requests

BASE = os.environ.get("BASE", "http://localhost:28101/lipidspace/v1")

# 3 real datasets + 1 SUPPORT frame-only pseudo-sample. FLAT table. dsB is missing 'species' to
# exercise NA-label skipping (a missing label must NOT surface as an empty / 'Ø' / '"' category).
rows = [["Sample", "tissue", "species", "lipid", "quant"]]
data = {
    "dsA": ("plasma", "homo sapiens", {"PC 34:1": 120, "PC 36:2": 80, "SM 34:1;O2": 40, "PE 34:1": 15}),
    "dsB": ("brain",  "",             {"PC 34:1": 20,  "PC 36:2": 30, "SM 34:1;O2": 90, "PE 34:1": 60}),
    "dsC": ("plasma", "mus musculus", {"PC 34:1": 110, "PC 36:2": 75, "SM 34:1;O2": 45, "PE 34:1": 18}),
}
for ds, (tis, sp, lipids) in data.items():
    for lip, q in lipids.items():
        rows.append([ds, tis, sp, lip, str(q)])
# support frame-only: reference lipids, empty label cells
for lip in ["PC 30:0", "PC 30:1", "PC 32:0", "PC 40:6", "TAG 52:2", "LPC 16:0", "Cer 18:1;O2/24:0"]:
    rows.append(["SUPPORT", "", "", lip, "1"])

def q(field):
    return '"' + field.replace('"', '""') + '"' if field != "" else ""
table = "\n".join(",".join(q(c) for c in r) for r in rows)

req = {
    "TableType": "FLAT_TABLE",
    "TableColumnTypes": ["SampleColumn", "StudyVariableColumnNominal", "StudyVariableColumnNominal",
                         "LipidColumn", "QuantColumn"],
    "Table": table,
    "Modules": 5,
    "LabelVariable": "tissue",
    "FrameOnlySamples": ["SUPPORT"],
}
r = requests.post(f"{BASE}/atlas/build", json=req, timeout=600)
r.raise_for_status()
atlas = r.json()

names = atlas["datasets"]
labelvars = set()
for m in atlas.get("meta", []):
    labelvars.update(m.keys())

print("datasets      :", names)
print("label vars    :", sorted(labelvars))
print("meta          :", atlas.get("meta"))

failures = []
if any("SUPPORT" in n for n in names):
    failures.append("SUPPORT frame-only sample leaked into datasets")
if any(" - table_file" in n for n in names):
    failures.append("dataset names carry the ' - table_file' suffix")
if "Origin" in labelvars:
    failures.append("'Origin' REST temp-file pseudo-variable leaked into label variables")
NA = {'"', "Ø", "", "NA", "N/A", "nan", "NaN", "n/a"}
if any(v in NA for m in atlas.get("meta", []) for v in m.values()):
    failures.append("a missing/NA label leaked as a metadata value (should be skipped)")
if set(names) != {"dsA", "dsB", "dsC"}:
    failures.append(f"expected exactly dsA/dsB/dsC, got {names}")
else:
    by_name = {n: m for n, m in zip(names, atlas.get("meta", []))}
    if "species" in by_name["dsB"]:
        failures.append(f"dsB is missing 'species' but it surfaced: {by_name['dsB']}")
    if by_name["dsA"].get("species") != "homo sapiens":
        failures.append(f"dsA species not preserved: {by_name['dsA']}")

if failures:
    print("\nFAIL:")
    for f in failures:
        print("  -", f)
    sys.exit(1)
print("\nPASS: SUPPORT excluded, names clean, no Origin, no stray quotes")
