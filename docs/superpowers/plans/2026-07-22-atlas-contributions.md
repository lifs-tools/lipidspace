# Atlas Contributions Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Surface which lipids and modules drive an Atlas fit result (per-predicted-variable lipid attribution, top modules + exemplars, global class signatures, dominant lipids) by decomposing quantities the model already computes.

**Architecture:** Query-independent data (`module_exemplars`, `class_baseline`, `class_signatures`) is precomputed in `Atlas::build()` and serialized into the blob. Query-dependent `contributions` (dominant lipids + per-variable attribution) is computed at fit time from the query's per-lipid module-membership matrix, which `generate_fingerprint` now exposes. Three optional top-N request params control output sizes.

**Tech Stack:** C++17 / Qt (LipidSpace REST server, cpp-httplib, nlohmann::json), Docker (linux/amd64), Python 3 + requests for REST verification.

## Global Constraints

- **Docker build (verbatim):** `docker build --platform linux/amd64 -f LipidSpaceRest.docker -t docker.lifs-tools.org/lifs/lipidspace:contrib-test .` — the vendored `libraries/*/linux64` are x86_64, so the build **must** be `--platform linux/amd64`, and the Dockerfile is `LipidSpaceRest.docker` (not `Dockerfile`).
- **Test container port:** use **28102** (dev stack is on 28100; keep them separate). Always `docker rm -f lipidspace-contrib-test` before re-running.
- **gitignore quirk:** files under `lipidspace/` are gitignored on macOS — stage `lipidspace/atlas.h` with `git add -f`.
- **Fit request field naming:** request params are PascalCase (`TopNDominantLipids`, `TopNLipids`, `TopNModules`), consistent with `NumNeighbors`/`LabelVariables`. Response fields are snake_case (`dominant_lipids`, `by_variable`, `nn_distance`).
- **Param semantics:** absent → default (10 / 10 / 5); explicit `0` → that section is emitted empty; negative → coerced to the default at the REST layer (so `Atlas` methods only ever receive `>= 0`).
- **Exemplars per module:** fixed `E = 6` (revisit later).
- Reference frozen model internals (`docs/superpowers/specs/2026-07-22-atlas-contributions-design.md`) for the math.

---

### Task 1: Build-time contributions data (module exemplars, class baseline, class signatures)

**Files:**
- Modify: `lipidspace/atlas.h` (members + method decls)
- Modify: `src/atlas.cpp` (implement helpers, call in `build()`, extend `to_json`/`from_json`)
- Test: `docs/atlas-phase0/atlas_contributions_build_test.py` (create)

**Interfaces:**
- Produces: `Atlas::module_exemplars` (`vector<vector<string>>`, length K), `Atlas::class_baseline` (`vector<double>`, length K), `Atlas::class_signatures` (`map<string, map<string, vector<double>>>`), `Atlas::compute_module_exemplars(int)`, `Atlas::compute_class_signatures()`. Blob gains `module_exemplars`, `class_baseline`, `class_signatures`. These are consumed by Task 2's `attribute()`.

- [ ] **Step 1: Write the failing REST test**

Create `docs/atlas-phase0/atlas_contributions_build_test.py`:
```python
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
```

- [ ] **Step 2: Add members + method declarations to `lipidspace/atlas.h`**

Immediately after the `vector<int> order;` member (the end of the Global-overview block), add these three members:
```cpp
    // --- Contributions (build-time, query-independent) ---
    vector<vector<string>> module_exemplars;                    // K x E frame-lipid names nearest each center
    vector<double> class_baseline;                              // K: global mean fingerprint
    map<string, map<string, vector<double>>> class_signatures;  // variable -> value -> K mean fingerprint
```
Then, after the `void compute_overview();` declaration, add:
```cpp
    // Precompute build-time contribution data (exemplars per module, class baseline + signatures).
    void compute_module_exemplars(int exemplars_per_module = 6);
    void compute_class_signatures();
```

- [ ] **Step 3: Implement `compute_module_exemplars` and `compute_class_signatures` in `src/atlas.cpp`**

Add both functions immediately after `Atlas::compute_overview()` (before `Atlas::capture_transform`):
```cpp
void Atlas::compute_module_exemplars(int E) {
    module_exemplars.assign(K, vector<string>());
    if (frame.empty() || centers.rows < K) return;
    for (int k = 0; k < K; ++k) {
        vector<pair<double, string>> d;
        d.reserve(frame.size());
        for (auto &kv : frame) {
            const vector<double> &coord = kv.second;
            double s = 0.0;
            for (int c = 0; c < dims && c < (int)coord.size(); ++c) {
                double diff = coord[c] - centers(k, c);
                s += diff * diff;
            }
            d.push_back(make_pair(s, kv.first));
        }
        int e = min((int)d.size(), E);
        partial_sort(d.begin(), d.begin() + e, d.end());
        vector<string> names;
        for (int i = 0; i < e; ++i) names.push_back(d[i].second);
        module_exemplars[k] = names;
    }
}

void Atlas::compute_class_signatures() {
    int N = (int)fingerprints.size();
    class_baseline.assign(K, 0.0);
    class_signatures.clear();
    if (N == 0) return;
    for (int i = 0; i < N; ++i)
        for (int k = 0; k < K && k < (int)fingerprints[i].size(); ++k)
            class_baseline[k] += fingerprints[i][k];
    for (int k = 0; k < K; ++k) class_baseline[k] /= (double)N;

    map<string, map<string, int>> counts;
    for (int i = 0; i < N; ++i) {
        for (auto &kv : meta[i]) {
            vector<double> &sig = class_signatures[kv.first][kv.second];
            if ((int)sig.size() != K) sig.assign(K, 0.0);
            for (int k = 0; k < K && k < (int)fingerprints[i].size(); ++k) sig[k] += fingerprints[i][k];
            counts[kv.first][kv.second]++;
        }
    }
    for (auto &var : class_signatures)
        for (auto &val : var.second) {
            int c = counts[var.first][val.first];
            if (c > 0) for (int k = 0; k < K; ++k) val.second[k] /= (double)c;
        }
}
```

- [ ] **Step 4: Call the helpers in `Atlas::build()`**

In `src/atlas.cpp`, replace the two-line tail of `build()`:
```cpp
    compute_overview();
    capture_transform(ls);
```
with:
```cpp
    compute_module_exemplars(6);
    compute_class_signatures();
    compute_overview();
    capture_transform(ls);
```

- [ ] **Step 5: Serialize in `to_json` and `from_json`**

In `Atlas::to_json`, after `j["order"] = order;`, add:
```cpp
    // Contributions (build-time)
    json jme = json::array();
    for (auto &ex : module_exemplars) jme.push_back(ex);
    j["module_exemplars"] = jme;
    j["class_baseline"] = class_baseline;
    json jcs = json::object();
    for (auto &var : class_signatures) {
        json jv = json::object();
        for (auto &val : var.second) jv[val.first] = val.second;
        jcs[var.first] = jv;
    }
    j["class_signatures"] = jcs;
```
In `Atlas::from_json`, after the `order` read (end of the function), add:
```cpp
    module_exemplars.clear();
    if (j.contains("module_exemplars"))
        for (auto &ex : j["module_exemplars"]) module_exemplars.push_back(ex.get<vector<string>>());
    class_baseline = j.value("class_baseline", vector<double>());
    class_signatures.clear();
    if (j.contains("class_signatures"))
        for (auto it = j["class_signatures"].begin(); it != j["class_signatures"].end(); ++it)
            for (auto vt = it.value().begin(); vt != it.value().end(); ++vt)
                class_signatures[it.key()][vt.key()] = vt.value().get<vector<double>>();
```

- [ ] **Step 6: Build the image**

Run: `docker build --platform linux/amd64 -f LipidSpaceRest.docker -t docker.lifs-tools.org/lifs/lipidspace:contrib-test .`
Expected: build succeeds (exit 0). If it fails to link with `-lcppGoslin` "incompatible", the `--platform linux/amd64` flag was dropped — re-add it.

- [ ] **Step 7: Run the container and the test — verify PASS**

```bash
docker rm -f lipidspace-contrib-test 2>/dev/null
docker run -d --name lipidspace-contrib-test --platform linux/amd64 -p 28102:8888 \
  docker.lifs-tools.org/lifs/lipidspace:contrib-test --tmp_folder /tmp
until [ "$(curl -s -o /dev/null -w '%{http_code}' http://localhost:28102/actuator/health)" = "200" ]; do sleep 1; done
python3 docs/atlas-phase0/atlas_contributions_build_test.py
```
Expected: `PASS: build-time contribution data present`

- [ ] **Step 8: Commit**

```bash
git add -f lipidspace/atlas.h
git add src/atlas.cpp docs/atlas-phase0/atlas_contributions_build_test.py
git commit -m "feat(atlas): precompute module exemplars + class signatures into the blob

Co-Authored-By: Claude Opus 4.8 <noreply@anthropic.com>"
```

---

### Task 2: Fit-time contributions (per-lipid memberships, attribution, top-N params)

**Files:**
- Modify: `lipidspace/Matrix.h:108` (add optional `contributions` out-param)
- Modify: `src/Matrix.cpp:734-789` (`generate_fingerprint` fills `contributions`)
- Modify: `lipidspace/atlas.h` (extend `fingerprint_query`, `fingerprint_query_projected`, `fit`, `fit_projected`; declare `attribute`)
- Modify: `src/atlas.cpp` (thread through, implement `attribute`, wire into `fit`/`fit_projected`)
- Modify: `src/lipidspacerest.cpp` (parse `TopN*`, pass to `fit`/`fit_projected`)
- Test: `docs/atlas-phase0/atlas_contributions_fit_test.py` (create)

**Interfaces:**
- Consumes (Task 1): `Atlas::module_exemplars`, `Atlas::datasets`, `Atlas::fingerprints`.
- Produces: `Matrix::generate_fingerprint(..., Matrix* contributions=nullptr)`; `Atlas::fit`/`fit_projected` gain `int top_n_dominant=10, int top_n_lipids=10, int top_n_modules=5`; fit result gains a `contributions` object `{ dominant_lipids:[{lipid,weight}], by_variable:{ VAR:{prediction, lipids:[{lipid,score,share}], modules:[{module,score,exemplars:[]}] } } }`.

- [ ] **Step 1: Write the failing REST test**

Create `docs/atlas-phase0/atlas_contributions_fit_test.py`:
```python
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
```

- [ ] **Step 2: Extend `generate_fingerprint` to emit per-lipid contributions**

In `lipidspace/Matrix.h:108`, change the declaration to:
```cpp
    void generate_fingerprint(Matrix& centers, Array& weights, Array& fingerprint,
                              double s, bool soft, Matrix* contributions = nullptr);
```
In `src/Matrix.cpp`, update the signature at line 734 to match (add `, Matrix* contributions`). Immediately after `for (int k = 0; k < K; ++k) fingerprint[k] = 0.0;` add:
```cpp
    if (contributions) {
        contributions->reset(n_lipids, K);
        for (int i = 0; i < n_lipids; ++i)
            for (int k = 0; k < K; ++k) (*contributions)(i, k) = 0.0;
    }
```
In the soft branch, replace the accumulation loop body so it also records the per-lipid term:
```cpp
            if (sum_exp > 0) {
                for (int k = 0; k < K; ++k) {
                    double ck = weight * dists[k] / sum_exp;
                    fingerprint[k] += ck;
                    if (contributions) (*contributions)(i, k) = ck;
                }
            }
```
In the hard branch, after `fingerprint[best_k] += weight;` add:
```cpp
            if (contributions) (*contributions)(i, best_k) = weight;
```
In the final normalization block, after dividing `fingerprint`, also divide the contributions:
```cpp
    if (contributions && sum > 0) {
        for (int i = 0; i < n_lipids; ++i)
            for (int k = 0; k < K; ++k) (*contributions)(i, k) /= sum;
    }
```

- [ ] **Step 3: Extend the Atlas declarations in `lipidspace/atlas.h`**

Change the `fingerprint_query` / `fingerprint_query_projected` / `fit` / `fit_projected` declarations to:
```cpp
    bool fingerprint_query(const vector<string> &species, const Array &weights,
                           Array &out_fp, double &coverage,
                           vector<string> *placed_names = nullptr, Matrix *contributions = nullptr);

    bool fingerprint_query_projected(LipidSpace &ls, Lipidome *query,
                                     const vector<LipidAdduct*> &ref_lipids,
                                     Array &out_fp, double &coverage, int &n_projected,
                                     vector<string> *placed_names = nullptr, Matrix *contributions = nullptr);

    json fit(const vector<string> &species, const Array &weights, int k,
             const set<string> &label_vars = set<string>(),
             int top_n_dominant = 10, int top_n_lipids = 10, int top_n_modules = 5);

    json fit_projected(LipidSpace &ls, Lipidome *query,
                       const vector<LipidAdduct*> &ref_lipids, int k,
                       const set<string> &label_vars = set<string>(),
                       int top_n_dominant = 10, int top_n_lipids = 10, int top_n_modules = 5);
```
After the `fit_projected` declaration, add:
```cpp
    // Build the per-query contributions block (dominant lipids + per-variable attribution).
    json attribute(const vector<string> &lipid_names, Matrix &contributions, Array &fp,
                   const json &neighbors, const json &predictions,
                   int top_n_dominant, int top_n_lipids, int top_n_modules);
```

- [ ] **Step 4: Thread `placed_names`/`contributions` through `fingerprint_query` in `src/atlas.cpp`**

Update the signature (line 270) to match Step 3. Inside the `for (int r ...)` loop that fills `qm`/`qw`, add after `qw[r] = ...;`:
```cpp
        if (placed_names) placed_names->push_back(species[present[r]]);
```
Change the `generate_fingerprint` call to:
```cpp
    qm.generate_fingerprint(centers, qw, out_fp, bandwidth, soft, contributions);
```

- [ ] **Step 5: Thread through `fingerprint_query_projected`**

Update the signature (line 370) to match Step 3. Add a names vector alongside the coords. After `vector<double> weights_list;` add:
```cpp
    vector<string> names_list;
```
In the frame-hit branch (after `weights_list.push_back(w);` inside `if (it != frame.end())`) add `names_list.push_back(query->species[i]);`. In the projected branch (after `weights_list.push_back(w);` following `coords_list.push_back(coords);`) add `names_list.push_back(query->species[i]);`. Change the fingerprint call to:
```cpp
    qm.generate_fingerprint(centers, qw, out_fp, bandwidth, soft, contributions);
    if (placed_names) *placed_names = names_list;
```

- [ ] **Step 6: Implement `Atlas::attribute` in `src/atlas.cpp`**

Add after `Atlas::fit_projected` (near line 437):
```cpp
json Atlas::attribute(const vector<string> &lipid_names, Matrix &C, Array &fp,
                      const json &neighbors, const json &predictions,
                      int top_n_dominant, int top_n_lipids, int top_n_modules) {
    json out;
    int n = (int)lipid_names.size();
    int Kc = (int)fp.size();

    // dominant lipids: row sums of C are the compositional weights.
    json jdom = json::array();
    if (top_n_dominant > 0 && n > 0) {
        vector<pair<double, int>> w(n);
        for (int i = 0; i < n; ++i) {
            double s = 0.0;
            for (int k = 0; k < Kc && k < C.cols; ++k) s += C(i, k);
            w[i] = make_pair(s, i);
        }
        int td = min(top_n_dominant, n);
        partial_sort(w.begin(), w.begin() + td, w.end(), greater<pair<double, int>>());
        for (int r = 0; r < td; ++r) {
            json e; e["lipid"] = lipid_names[w[r].second]; e["weight"] = w[r].first;
            jdom.push_back(e);
        }
    }
    out["dominant_lipids"] = jdom;

    json byvar = json::object();
    for (auto it = predictions.begin(); it != predictions.end(); ++it) {
        const string var = it.key();
        string pred = it.value().value("prediction", string());

        // voting neighbours = those carrying var == pred; collect their fingerprints.
        vector<Array*> qs;
        for (auto &nb : neighbors) {
            if (!nb.contains("metadata")) continue;
            const json &md = nb["metadata"];
            if (!md.contains(var) || md[var].get<string>() != pred) continue;
            string dname = nb.value("dataset", string());
            for (int di = 0; di < (int)datasets.size(); ++di)
                if (datasets[di] == dname) { qs.push_back(&fingerprints[di]); break; }
        }

        vector<double> BC(Kc, 0.0);
        for (Array *q : qs)
            for (int k = 0; k < Kc && k < (int)q->size(); ++k)
                BC[k] += sqrt(fp[k] * (*q)[k]);

        json jmods = json::array();
        if (top_n_modules > 0) {
            vector<pair<double, int>> mk(Kc);
            for (int k = 0; k < Kc; ++k) mk[k] = make_pair(BC[k], k);
            int tm = min(top_n_modules, Kc);
            partial_sort(mk.begin(), mk.begin() + tm, mk.end(), greater<pair<double, int>>());
            for (int r = 0; r < tm; ++r) {
                int k = mk[r].second;
                json m; m["module"] = k; m["score"] = mk[r].first;
                m["exemplars"] = (k < (int)module_exemplars.size()) ? module_exemplars[k] : vector<string>();
                jmods.push_back(m);
            }
        }

        json jlips = json::array();
        if (top_n_lipids > 0 && n > 0) {
            vector<double> contrib(n, 0.0);
            double total = 0.0;
            for (int i = 0; i < n; ++i) {
                double s = 0.0;
                for (int k = 0; k < Kc && k < C.cols; ++k)
                    if (fp[k] > 0.0) s += C(i, k) * (BC[k] / fp[k]);
                contrib[i] = s; total += s;
            }
            vector<pair<double, int>> ci(n);
            for (int i = 0; i < n; ++i) ci[i] = make_pair(contrib[i], i);
            int tl = min(top_n_lipids, n);
            partial_sort(ci.begin(), ci.begin() + tl, ci.end(), greater<pair<double, int>>());
            for (int r = 0; r < tl; ++r) {
                int i = ci[r].second;
                json e; e["lipid"] = lipid_names[i]; e["score"] = contrib[i];
                e["share"] = total > 0.0 ? contrib[i] / total : 0.0;
                jlips.push_back(e);
            }
        }

        json v; v["prediction"] = pred; v["lipids"] = jlips; v["modules"] = jmods;
        byvar[var] = v;
    }
    out["by_variable"] = byvar;
    return out;
}
```

- [ ] **Step 7: Wire `attribute` into `fit` and `fit_projected`**

Replace `Atlas::fit` (lines 356-367) with:
```cpp
json Atlas::fit(const vector<string> &species, const Array &weights, int k,
                const set<string> &label_vars,
                int top_n_dominant, int top_n_lipids, int top_n_modules) {
    Array fp;
    double coverage = 0.0;
    vector<string> placed;
    Matrix C;
    if (!fingerprint_query(species, weights, fp, coverage, &placed, &C)) {
        json result;
        result["error"] = "no query lipids fell in the frozen frame";
        result["coverage"] = coverage;
        return result;
    }
    json result = rank(fp, coverage, k, label_vars);
    result["contributions"] = attribute(placed, C, fp, result["neighbors"], result["predictions"],
                                         top_n_dominant, top_n_lipids, top_n_modules);
    return result;
}
```
Replace `Atlas::fit_projected` (lines 422-437) with:
```cpp
json Atlas::fit_projected(LipidSpace &ls, Lipidome *query,
                          const vector<LipidAdduct*> &ref_lipids, int k,
                          const set<string> &label_vars,
                          int top_n_dominant, int top_n_lipids, int top_n_modules) {
    Array fp;
    double coverage = 0.0;
    int n_projected = 0;
    vector<string> placed;
    Matrix C;
    if (!fingerprint_query_projected(ls, query, ref_lipids, fp, coverage, n_projected, &placed, &C)) {
        json result;
        result["error"] = "no query lipids could be placed in the frame";
        result["coverage"] = coverage;
        return result;
    }
    json r = rank(fp, coverage, k, label_vars);
    r["projected_lipids"] = n_projected;
    r["contributions"] = attribute(placed, C, fp, r["neighbors"], r["predictions"],
                                    top_n_dominant, top_n_lipids, top_n_modules);
    return r;
}
```

- [ ] **Step 8: Parse the top-N params in `src/lipidspacerest.cpp`**

In the `/atlas/fit` handler, locate where `k` (NumNeighbors) and `label_vars` are parsed from the request `body`. Immediately after, add:
```cpp
            int top_n_dominant = body.value("TopNDominantLipids", 10);
            int top_n_lipids   = body.value("TopNLipids", 10);
            int top_n_modules  = body.value("TopNModules", 5);
            if (top_n_dominant < 0) top_n_dominant = 10;
            if (top_n_lipids   < 0) top_n_lipids   = 10;
            if (top_n_modules  < 0) top_n_modules  = 5;
```
Then update the two fit calls (the `project ? ... : ...` expression) to pass them:
```cpp
                json r = project
                    ? atlas.fit_projected(lipid_space, lipidome, ref_lipids, k, label_vars,
                                          top_n_dominant, top_n_lipids, top_n_modules)
                    : atlas.fit(lipidome->species, lipidome->original_intensities, k, label_vars,
                                top_n_dominant, top_n_lipids, top_n_modules);
```
(Match the exact `body` variable name used by the handler; if it differs, use that name.)

- [ ] **Step 9: Rebuild the image**

Run: `docker build --platform linux/amd64 -f LipidSpaceRest.docker -t docker.lifs-tools.org/lifs/lipidspace:contrib-test .`
Expected: build succeeds (exit 0).

- [ ] **Step 10: Run both REST tests — verify PASS**

```bash
docker rm -f lipidspace-contrib-test 2>/dev/null
docker run -d --name lipidspace-contrib-test --platform linux/amd64 -p 28102:8888 \
  docker.lifs-tools.org/lifs/lipidspace:contrib-test --tmp_folder /tmp
until [ "$(curl -s -o /dev/null -w '%{http_code}' http://localhost:28102/actuator/health)" = "200" ]; do sleep 1; done
python3 docs/atlas-phase0/atlas_contributions_build_test.py
python3 docs/atlas-phase0/atlas_contributions_fit_test.py
```
Expected: both print `PASS`.

- [ ] **Step 11: Commit**

```bash
git add -f lipidspace/atlas.h
git add lipidspace/Matrix.h src/Matrix.cpp src/atlas.cpp src/lipidspacerest.cpp \
        docs/atlas-phase0/atlas_contributions_fit_test.py
git commit -m "feat(atlas): per-query contributions (dominant lipids + prediction attribution)

Co-Authored-By: Claude Opus 4.8 <noreply@anthropic.com>"
```

---

### Task 3: Document the new API surface

**Files:**
- Modify: `examples/Rest/lipidspace-openapi.yml`
- Modify: `docs/atlas-guide.md`

**Interfaces:**
- Consumes: the request params + response/blob fields from Tasks 1–2. No code; documentation only.

- [ ] **Step 1: Add request params + `contributions` + blob fields to the OpenAPI spec**

In `examples/Rest/lipidspace-openapi.yml`, under the `/atlas/fit` request body schema properties (alongside `NumNeighbors`, `LabelVariables`), add:
```yaml
                                    TopNDominantLipids:
                                        type: integer
                                        default: 10
                                        description: Number of top-abundance query lipids to return (0 = omit).
                                    TopNLipids:
                                        type: integer
                                        default: 10
                                        description: Number of top attributing lipids per predicted variable (0 = omit).
                                    TopNModules:
                                        type: integer
                                        default: 5
                                        description: Number of top driving modules per predicted variable (0 = omit).
```
Under the fit **response** result schema, add a `contributions` object property:
```yaml
                                    contributions:
                                        type: object
                                        description: Dominant lipids and per-predicted-variable attribution (lipids + modules).
```
Under the `/atlas/build` response (near `embedding`/`order`), add:
```yaml
                                    module_exemplars:
                                        type: array
                                        description: Per module, the frame lipids nearest its center (K lists).
                                        items: { type: array, items: { type: string } }
                                    class_baseline:
                                        type: array
                                        description: Global mean fingerprint (K values).
                                        items: { type: number }
                                    class_signatures:
                                        type: object
                                        description: Per nominal variable/value mean fingerprint (the class signature).
```

- [ ] **Step 2: Document in the guide**

In `docs/atlas-guide.md`, add a "Contributions" subsection to §4 (`POST /atlas/fit`) describing: `TopNDominantLipids`/`TopNLipids`/`TopNModules` request params (defaults 10/10/5, 0 = omit) and the `contributions` response block (`dominant_lipids`, `by_variable[VAR]` with `lipids`/`modules`), plus a one-line note that `module_exemplars` / `class_signatures` / `class_baseline` are added to the build artifact and back the global class signature.

- [ ] **Step 3: Commit**

```bash
git add examples/Rest/lipidspace-openapi.yml docs/atlas-guide.md
git commit -m "docs(atlas): document contributions params, response, and blob fields

Co-Authored-By: Claude Opus 4.8 <noreply@anthropic.com>"
```

---

### Task 4: Live end-to-end verification on the dev stack

**Files:** none (integration check).

**Interfaces:** Consumes the running LipidCompass dev stack + `dev-auth.sh`.

- [ ] **Step 1: Publish the fixed image to the dev-stack tag**

```bash
docker tag docker.lifs-tools.org/lifs/lipidspace:contrib-test docker.lifs-tools.org/lifs/lipidspace:1.2.1
```
Then ask the user to recreate the `lipidspace` service so the container adopts the new image (as in prior rounds). Confirm the running container's image id matches `docker images -q docker.lifs-tools.org/lifs/lipidspace:1.2.1`.

- [ ] **Step 2: Force an atlas rebuild (so the blob gains the new build-time fields)**

```bash
cd <lipidcompass main checkout>
ADMIN_JWT=$(KC_PASSWORD='<pw>' ./dev-auth.sh -u lifsadmin)
curl -s -X POST 'http://127.0.0.1:8081/atlas/rebuild?force=true' -H "Authorization: Bearer $ADMIN_JWT" \
  | python3 -c 'import sys,json;d=json.load(sys.stdin);print(d["status"], d["datasetCount"])'
```
Expected: `READY 129` (or the current dataset count).

- [ ] **Step 3: Verify contributions on a real dataset via LipidSpace directly**

The backend `AtlasFitResult` DTO does not surface `contributions` yet (deferred), but the LipidSpace container does. Fit a query against the dev atlas by calling the LipidSpace container's `/atlas/fit` with the freshly-built atlas, or re-run `docs/atlas-phase0/atlas_rest_test.py` against `http://localhost:28100/lipidspace/v1` and confirm each result now carries a non-empty `contributions.by_variable` with plausible plasma-characteristic top lipids and module exemplars. Record the observation.

- [ ] **Step 4: Clean up the local test container**

```bash
docker rm -f lipidspace-contrib-test 2>/dev/null
```

---

## Self-Review

**Spec coverage:** view 1 (per-variable `lipids` — Task 2 Step 6); view 2 (`modules` + exemplars — Task 1 Step 3 exemplars, Task 2 Step 6 ranking); view 3 (`class_signatures`/`class_baseline` — Task 1); view 4 (`dominant_lipids` — Task 2 Step 6). Request params (Task 2 Step 8). Attribution math (Task 2 Step 6). Serialization (Task 1 Step 5). Docs (Task 3). Live verify (Task 4). Testing incl. perturbation sanity (Task 2 Step 1). All spec sections covered.

**Placeholder scan:** no "TBD/TODO/handle edge cases" placeholders; every code step shows concrete code and every test step shows the full test.

**Type consistency:** `contributions` (`Matrix*`) is the same param name across `generate_fingerprint`, `fingerprint_query`, `fingerprint_query_projected`; `attribute(lipid_names, C, fp, neighbors, predictions, top_n_dominant, top_n_lipids, top_n_modules)` signature matches its two call sites in `fit`/`fit_projected`; `module_exemplars`/`class_baseline`/`class_signatures` names are identical in `atlas.h`, `compute_*`, `to_json`/`from_json`, and `attribute`.
