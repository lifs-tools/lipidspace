# Building and using a LipidSpace Atlas

The **Atlas** turns LipidSpace's structural lipid-distance space into an incremental, queryable
background model of lipidome datasets. You build it once from a set of datasets (optionally over
a broad reference frame), persist the resulting artifact, and then *fit* any new lipidome against
it to find where it sits — with a calibrated confidence and an out-of-distribution flag.

This guide is for any REST client. LipidCompass is one consumer; the endpoints are generic.

- **Base URL** (local docker): `http://localhost:8888/lipidspace/v1`
- **Interactive docs**: `GET /lipidspace/v1/docs` (Swagger UI) · spec at `GET /lipidspace/v1/openapi.yaml`

---

## 1. Concepts

| Piece | What it is |
|---|---|
| **Frame** | Each lipid's coordinates in the PCA of the L×L structural (Tanimoto) distance matrix. Frozen once at build time. |
| **Modules** | `K` k-means regions over the frame (default `K=20`). |
| **Fingerprint** | Per dataset: an abundance-weighted, compositionally-normalised histogram over the `K` modules. Fixed length, so datasets are cheap to compare and index. |
| **Distance** | Hellinger distance between fingerprints (a metric in `[0,1]`). |
| **Transform** | The frame's PCA transform (column mean/stdev + eigenvectors), stored so a lipid **not** in the frame can be **Nyström-projected** into it from its distance row to the reference lipids, instead of being dropped. |
| **Calibration** | The distribution of each dataset's nearest-neighbour distance, used to turn a fit into a `confidence` in `[0,1]` and an `ood` flag. |

The whole artifact is portable JSON (a few MB). You persist it and pass it back on every `/atlas/fit`
call (see [§6 gzip](#6-gzip) for keeping that cheap).

---

## 2. Lifecycle

```
                 build once (per refit)                 fit many (per query)
  datasets  ─────────────────────────────►  atlas JSON  ─────────────────────►  nearest datasets
  (+ support frame)                          (persist)                          + confidence + OOD
```

- **Build** is `O(datasets)` after the one-time `O(L²)` frame. Refit on your own cadence (e.g. after
  each import, or every few days).
- **Fit** is `O(query size + N datasets)` and never mutates the atlas.
- A dataset can be *added* later by fingerprinting it alone against the stored frame — no refit of the
  frame or modules is required. (The `/atlas/add` convenience endpoint is planned; today, rebuild.)

---

## 3. `POST /atlas/build`

Builds the atlas and returns it as JSON. Reuses the `/pca` table fields.

### Request

| Field | Type | Notes |
|---|---|---|
| `TableType` | string | `FLAT_TABLE` (recommended for large/sparse inputs), `COLUMN_PIVOT_TABLE`, or `ROW_PIVOT_TABLE`. |
| `TableColumnTypes` | string[] | One per column: `SampleColumn`, `LipidColumn`, `QuantColumn`, `StudyVariableColumnNominal`, `StudyVariableColumnNumerical`, `IgnoreColumn`. |
| `Table` | string | The table as a CSV string. **Quote every field** — canonical lipid names contain commas (e.g. `Cer 18:0;1OH,3OH/22:0`). |
| `Modules` | int | Optional, default `20`. Number of k-means modules `K`. |
| `LabelVariable` | string | Optional. A nominal study-variable name (e.g. `"tissue"`) used for the majority-vote prediction at fit time. |
| `FrameOnlySamples` | string[] | Optional. Samples that **define the frame** but are excluded from the stored datasets and calibration — see [§5 support frame](#5-building-a-support-frame). |

Requires `Content-Type: application/json` and at least 3 lipid species.

### Response

The atlas artifact. Notable fields:

| Field | Meaning |
|---|---|
| `frame` | `{ canonicalLipidName: [coord…] }` |
| `centers`, `bandwidth` | module centres and soft-assignment temperature |
| `datasets`, `fingerprints`, `meta` | per-dataset name, fingerprint, and nominal study variables |
| `nn_ref`, `ood_threshold` | calibration |
| `ref_names`, `col_mean`, `col_inv_stdev`, `eigenvectors` | the Nyström projection transform |
| `embedding`, `order` | global overview: N×2 classical-MDS coords of the datasets + an average-linkage clustering leaf order (for an "atlas explorer" map + clustermap) |
| `roundtrip_error` | build self-check: max &#124;reprojected − stored&#124; frame coord. Expect ~`1e-12`. A large value means the transform is inconsistent — treat the atlas as suspect. |

### Minimal example (COLUMN_PIVOT_TABLE)

```bash
curl -sS http://localhost:8888/lipidspace/v1/atlas/build \
  -H 'Content-Type: application/json' -H 'Accept-Encoding: gzip' --compressed \
  -d '{
    "TableType": "COLUMN_PIVOT_TABLE",
    "TableColumnTypes": ["SampleColumn","StudyVariableColumnNominal","LipidColumn","LipidColumn","LipidColumn"],
    "Table": "\"Sample\",\"tissue\",\"PC 34:1\",\"PC 36:2\",\"SM 34:1;O2\"\n\"studyA\",\"plasma\",\"120\",\"80\",\"40\"\n\"studyB\",\"brain\",\"20\",\"30\",\"90\"",
    "Modules": 20,
    "LabelVariable": "tissue"
  }' > atlas.json
```

---

## 4. `POST /atlas/fit`

Fingerprints one or more query lipidomes against a prebuilt atlas.

### Request

| Field | Type | Notes |
|---|---|---|
| `Atlas` | object | An atlas artifact from `/atlas/build`. |
| `TableType`, `TableColumnTypes`, `Table` | | The query table (same encoding as build). No study-variable columns needed — the labels are predicted. |
| `NumNeighbors` | int | Optional, default `5`. Neighbours returned / voted over. |
| `LabelVariables` | string[] | Optional. Which nominal study variables to predict, e.g. `["tissue","species","disease"]`. Omit to predict **every** nominal variable stored in the atlas (each dataset can carry many — tissue, species, disease, cell type, custom CV terms). |
| `TopNDominantLipids` | int | Optional, default `10`. Size of `contributions.dominant_lipids` (`0` = omit). |
| `TopNLipids` | int | Optional, default `10`. Attributing lipids per predicted variable (`0` = omit). |
| `TopNModules` | int | Optional, default `5`. Driving modules per predicted variable (`0` = omit). |

### Response — one entry per query lipidome

| Field | Meaning |
|---|---|
| `query` | query sample name |
| `neighbors[]` | `{ dataset, distance, metadata }`, nearest first |
| `predictions` | `{ variable: { prediction, vote } }` — a majority-vote label + fraction for each predicted nominal study variable (tissue, species, disease, …). |
| `prediction`, `vote` | convenience echo of `predictions[LabelVariable]` for the build-time default label variable. |
| `nn_distance` | distance to the nearest dataset |
| `confidence` | in `[0,1]`: the fraction of atlas datasets whose own nearest neighbour is **farther** than this query's. High = this query sits as tightly as the tightest datasets. |
| `ood` | `true` if `nn_distance` exceeds the calibrated threshold (95th percentile of `nn_ref`) — treat as out-of-distribution. |
| `coverage` | fraction of query lipids placed in the frame (looked up **or** projected). |
| `projected_lipids` | how many were placed by Nyström projection rather than direct lookup. |

**Recommended usage:** act on `prediction`/`neighbors` when `confidence` is high and `ood` is false;
surface the rest as "uncertain" rather than returning a confidently-wrong answer.

### Example

```bash
# atlas.json is the artifact from /atlas/build
jq -n --slurpfile a atlas.json '{
  Atlas: $a[0],
  TableType: "COLUMN_PIVOT_TABLE",
  TableColumnTypes: ["SampleColumn","LipidColumn","LipidColumn","LipidColumn"],
  Table: "\"Sample\",\"PC 34:1\",\"PC 36:2\",\"SM 34:1;O2\"\n\"myquery\",\"110\",\"75\",\"45\"",
  NumNeighbors: 5
}' | curl -sS http://localhost:8888/lipidspace/v1/atlas/fit \
      -H 'Content-Type: application/json' --data-binary @- | jq .results[0]
```

### Contributions

Each result also carries a `contributions` block explaining *why* the query landed where it did. It is
decomposed from the same fingerprint, so the attribution is exact rather than a heuristic:

| Field | Meaning |
|---|---|
| `dominant_lipids` | `[{ lipid, weight }]` — the query's highest-abundance species (compositional weight). |
| `by_variable` | `{ variable: { prediction, lipids, modules } }` — one entry per predicted variable (mirrors `predictions`). |
| `by_variable[V].lipids` | `[{ lipid, score, share }]` — the query lipids that most drove `V`'s prediction; `share` sums to ~1. |
| `by_variable[V].modules` | `[{ module, score, exemplars }]` — the modules driving the match, each with representative frame lipids. |

Sizes are controlled by `TopNDominantLipids` / `TopNLipids` / `TopNModules` (defaults `10` / `10` / `5`; `0` omits a section).

The build artifact additionally carries `module_exemplars` (the frame lipids nearest each module centre),
`class_baseline` (the global mean fingerprint), and `class_signatures` (per label value mean fingerprint —
the global signature of e.g. `tissue = plasma`, read against `class_baseline`).

---

## 5. Building a support frame

By default the frame is the union of the datasets' own lipids — biased toward what was measured.
To seed a **stable, broad structural background** (so most query lipids land in the frame directly),
add a *frame-only* pseudo-sample carrying a reference set and name it in `FrameOnlySamples`. Its lipids
define the PCA space and modules but it is **not** stored as a dataset or used for calibration.

Use a sparse `FLAT_TABLE` (`Sample, [study vars], lipid, quant` per row) so a large reference set costs
one row per lipid instead of a huge dense column set.

```
Sample,tissue,lipid,quant
studyA,plasma,PC 34:1,120
studyA,plasma,PC 36:2,80
...                                     ← corpus datasets (their lipids + abundances + label)
SUPPORT,support,PC 30:0,1
SUPPORT,support,PC 30:1,1
...                                     ← one row per reference lipid, quant = 1
```

```jsonc
{
  "TableType": "FLAT_TABLE",
  "TableColumnTypes": ["SampleColumn","StudyVariableColumnNominal","LipidColumn","QuantColumn"],
  "Table": "<the CSV above, every field quoted>",
  "Modules": 20,
  "LabelVariable": "tissue",
  "FrameOnlySamples": ["SUPPORT"]
}
```

**Choosing the reference set.** Species-level shorthand names from SwissLipids and/or LIPID MAPS work
well. Cost is `O(L²)` in the frame size `L` (Tanimoto + PCA), so it bounds how large you go:

| Reference set | ~lipids | Frame (∪ corpus) | Build (emulated) | Peak RAM |
|---|---|---|---|---|
| LIPID MAPS species | 3.2k | ~4.8k | ~25 s | < 1 GB |
| SwissLipids ∪ LIPID MAPS species | ~11k | ~11k | ~2.5 min | ~4 GB |

Lipids in your datasets or queries that fall outside the frame are **Nyström-projected**, so coverage
stays complete regardless of the reference set — a broader frame simply means fewer projections and
sharper module resolution.

---

## 6. gzip

The artifact is JSON, so gzip it wherever you keep or move it.

- **Storage:** persist the atlas as **gzipped JSON** (`atlas.json.gz`). ~4 MB → ~1 MB. Key it by an id
  you control (e.g. a database version) so you can invalidate/rebuild it on a schedule.
- **Transport (responses):** send `Accept-Encoding: gzip` on `/atlas/build` and the server returns the
  artifact gzip-compressed on the wire (`curl --compressed` handles this transparently).
- **Transport (requests):** `/atlas/fit` carries the whole atlas in the body. Send it gzipped with
  `Content-Encoding: gzip` and the server decompresses it:

  ```bash
  gzip -c fit-request.json | curl -sS http://localhost:8888/lipidspace/v1/atlas/fit \
      -H 'Content-Type: application/json' -H 'Content-Encoding: gzip' --data-binary @-
  ```

gzip is opt-in per request (via the headers), so existing clients are unaffected.

---

## 7. Notes & guidance

- **Quote every table field.** Canonical lipid names contain commas and semicolons.
- **Refit cadence.** The frame + transform change only when you rebuild; per-dataset fingerprints are
  what grow. Rebuilding is idempotent — same input, same atlas (k-means is seeded).
- **Confidence over accuracy.** Cross-lab/batch effects are intrinsic to real lipidome collections; the
  confidence/OOD signals are the safeguard. Prefer "return the confident, flag the rest".
- **`roundtrip_error`** should be ~`1e-12`. If it isn't, do not trust projections from that atlas.
- The exact request/response schemas are in `examples/Rest/lipidspace-openapi.yml` and the Phase 0
  evaluation + reference harness live under `docs/atlas-phase0/`.
```
