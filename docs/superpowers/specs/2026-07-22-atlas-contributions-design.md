# Atlas contributions: dominant lipids & prediction attribution

**Status:** design · **Scope:** LipidSpace only (backend/client/UI wire-through deferred to the LipidCompass frontend phase)

## 1. Goal

From the *existing* Atlas model, surface **which lipids and modules drive a fit result** — without any new modelling, purely by decomposing quantities the model already computes. Four views, all requested:

1. **Per-prediction lipid attribution** — the query lipids most responsible for each predicted label.
2. **Top modules + exemplar lipids** — the modules driving each prediction, made readable by their representative frame lipids.
3. **Global class signature** — what characterizes each label value (e.g. `tissue = blood plasma`) across all datasets, independent of any query.
4. **Dominant lipids** — the highest-abundance species in the query (a raw companion).

## 2. Why it decomposes cleanly

A fingerprint is a **sum of per-lipid contributions**. For query lipid `i` with compositional weight `wᵢ` and soft module membership `aᵢₖ` (Σₖ aᵢₖ = 1):

```
cᵢₖ = wᵢ · aᵢₖ          (contribution of lipid i to module k)
pₖ  = Σᵢ cᵢₖ            (the query fingerprint; Σₖ pₖ = 1)
```

Neighbour similarity is the Bhattacharyya coefficient `BC(p,q) = Σₖ √(pₖ qₖ)` (Hellinger² = 1 − BC). Both the fingerprint and the similarity are therefore linear/separable in the modules, which makes attribution **exact**, not a heuristic.

## 3. Architecture — split by query-dependence

Mirrors the existing `embedding`/`order` precompute-at-build vs. compute-at-fit split.

### Build time → stored in the atlas blob (query-independent)
- `module_exemplars[k]` — the `E` (default 6) frame lipids nearest module `k`'s centre, by Euclidean distance in frame space. Names only.
- `class_baseline` — the global mean fingerprint over all stored datasets (K doubles).
- `class_signatures[variable][value]` — the mean fingerprint over the datasets carrying that nominal label value (K doubles). NA/missing values are already excluded from `meta`, so they never form a class. This is view 3; lift vs. `class_baseline` is derived downstream.

### Fit time → per-query `contributions` block
- `dominant_lipids` — top-`TopNDominantLipids` query lipids by compositional weight (view 4).
- `by_variable[V]` — for each **predicted** nominal variable `V` (mirrors the existing `predictions` map): its attribution over the neighbours that carry `V`'s predicted value (views 1 & 2).

## 4. Algorithms

Let the fit use the `k` nearest neighbour datasets (`NumNeighbors`), each with stored fingerprint `q⁽ʲ⁾` and metadata.

### 4.1 Dominant lipids (view 4)
Normalize the query's placed-lipid weights to sum to 1; sort desc; take `TopNDominantLipids`. Emit `{lipid, weight}` (weight = normalized fraction).

### 4.2 Per-variable attribution (views 1 & 2)
For predicted variable `V` with majority-vote value `v*`:
- Voting set `N = { j : meta[j][V] == v* }` (non-empty, since `v*` is the majority).
- Per-module drive: `BCₖ = Σ_{j∈N} √(pₖ · q⁽ʲ⁾ₖ)`.
- **Modules** (view 2): rank `k` by `BCₖ` desc, take `TopNModules`; emit `{module:k, score:BCₖ, exemplars: module_exemplars[k]}`.
- **Lipids** (view 1): `contribᵢ = Σ_{k: pₖ>0} cᵢₖ · (BCₖ / pₖ)`. Rank `i` desc, take `TopNLipids`; `shareᵢ = contribᵢ / Σᵢ contribᵢ`. Emit `{lipid, score:contribᵢ, share:shareᵢ}`.

`cᵢₖ > 0 ⟹ pₖ ≥ cᵢₖ > 0`, so the division is always defined and bounded (`cᵢₖ/pₖ ≤ 1`, `BCₖ` bounded) — no small-`pₖ` blow-up.

### 4.3 Per-lipid memberships
Attribution needs the query's per-lipid contribution matrix `C = [cᵢₖ]`. The fingerprint routine already computes `aᵢₖ` internally; extend it to optionally emit `C` (an out-parameter), and have `fingerprint_query` / `fingerprint_query_projected` surface it. Projected (Nyström) query lipids have frame coords too, so they attribute normally.

### 4.4 Build-time precompute
- `module_exemplars`: for each centre row `k`, the `E` nearest frame lipids by Euclidean distance.
- `class_baseline`: mean of all dataset fingerprints.
- `class_signatures`: group datasets by each nominal `meta` variable/value; mean fingerprint per group.

## 5. Data structures

### 5.1 Fit request — new optional fields (PascalCase, per existing request convention)
| Field | Type | Default | Meaning |
|---|---|---|---|
| `TopNDominantLipids` | int | 10 | length of `dominant_lipids` |
| `TopNLipids` | int | 10 | length of each variable's `lipids` |
| `TopNModules` | int | 5 | length of each variable's `modules` |

Absent → default. Explicit `0` → that section is emitted empty (opt-out). Negative → treated as default.

### 5.2 Fit response — new `contributions` per result (snake_case, per existing response convention)
```jsonc
"contributions": {
  "dominant_lipids": [ {"lipid":"PC 34:1","weight":0.08}, ... ],
  "by_variable": {
    "tissue": {
      "prediction": "blood plasma",
      "lipids":  [ {"lipid":"PC 34:1","score":0.31,"share":0.22}, ... ],
      "modules": [ {"module":7,"score":0.18,"exemplars":["PC 34:1","PC 36:2"]}, ... ]
    },
    "species": { "prediction":"Homo sapiens", "lipids":[...], "modules":[...] }
  }
}
```
`by_variable` keys == `predictions` keys (the selected/available predicted variables).

### 5.3 Atlas blob — new fields
```jsonc
"module_exemplars": [ ["PC 34:1","PC 36:2", ...], ... ],   // length K
"class_baseline":   [k0, k1, ..., k19],                    // K doubles
"class_signatures": { "tissue": { "blood plasma":[K doubles], ... }, "species": {...} }
```

## 6. Implementation surface (C++)

- **`lipidspace/atlas.h`** — new members (`module_exemplars`, `class_baseline`, `class_signatures`); decls for `compute_module_exemplars()`, `compute_class_signatures()`, and an attribution helper; `to_json`/`from_json` additions.
- **`src/atlas.cpp`** — `build()` calls the two precompute helpers; `rank()`/`fit()` build the `contributions` json (dominant lipids + per-variable attribution) using the query's `C` and the neighbours already selected. Honors the three top-N params.
- **`lipidspace/Matrix.*`** — `generate_fingerprint` gains an optional per-lipid contribution out-parameter (`C`), leaving existing callers unchanged.
- **`src/lipidspacerest.cpp`** — parse `TopNDominantLipids`/`TopNLipids`/`TopNModules` from the fit body (defaults when absent); thread through to `fit`.
- **`examples/Rest/lipidspace-openapi.yml`**, **`docs/atlas-guide.md`** — document the new request params, `contributions` response, and blob fields.

## 7. Testing & verification

- **`docs/atlas-phase0/atlas_contributions_test.py`** (REST, like `atlas_frameonly_test.py`): build a labeled atlas, fit a query, assert:
  - `dominant_lipids` sorted by weight desc, length ≤ `TopNDominantLipids`.
  - `by_variable` has one entry per predicted variable; each `prediction` equals `predictions[V].prediction`.
  - `lipids` sorted by score desc, `Σ share ≈ 1`, length ≤ `TopNLipids`; `modules` length ≤ `TopNModules` with non-empty `exemplars`.
  - top-N params are honored (set them and check lengths; `0` ⇒ empty).
  - **Perturbation sanity:** removing the #1 lipid for variable `V` increases Hellinger distance to `V`'s predicted-class nearest neighbour (analytic attribution agrees with a leave-one-out perturbation, direction check).
- **Live:** fit `LCS-00001-1` against the dev-stack atlas (129 datasets); confirm plasma-characteristic top lipids and sensible module exemplars.

## 8. Edge cases
- Query lipid absent from the frame → Nyström-projected → still attributes.
- A variable whose predicted value has a single voting neighbour → attribution over that one.
- `TopN*` larger than available → return all.
- Class value with one dataset → signature is that dataset's fingerprint.
- Modules with zero query mass → excluded naturally (`BCₖ = 0`).

## 9. Out of scope (deferred to the LipidCompass frontend phase)
OpenAPI/`jlipidspace` regen + redeploy, the backend `AtlasFitResult` DTO fields, backend surfacing of `contributions`/`class_signatures`, and the UI. The backend client already tolerates unknown response fields (`FAIL_ON_UNKNOWN_PROPERTIES=false`), so shipping the new fit fields now does not break the running backend — they are simply ignored until the DTO is extended.
