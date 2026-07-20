# Atlas — Phase 0 validation harness

Evidence that a fixed-length **structural fingerprint** over LipidSpace's frame recovers
lipidome biology (tissue / species) at least as well as the Hausdorff distance, while
being incremental. See [`../atlas-design.html`](../atlas-design.html) §10 and
[`REPORT.md`](REPORT.md) for the verdict.

## Pipeline

| step | script | output |
|---|---|---|
| 1 | `ingest.py`   | parse the mzTab-M corpus → per-dataset relative-composition profiles + species/tissue → `corpus.json` |
| 2 | `lsclient.py` | one LipidSpace `/pca` call → frozen 2D frame + Hausdorff ground truth → `lsresult.json` |
| 3 | `evaluate.py` | k-means modules, compositional fingerprints, LODO metrics, parameter sweep |

`module_alloc_preview.py` separately previews the COMP_DB-based per-class module
allocation (needs `COMP_DB_DATA.tsv` from LIPID MAPS).

## Requirements
- Python 3.12 with `numpy`, `scipy`, `scikit-learn`, `requests`.
- A running LipidSpace REST server — default `http://localhost:28100/lipidspace/v1/pca`
  (override with the `URL` env var).
- The mzTab-M study corpus (path hard-coded as `STUDIES` in `ingest.py`).

## Run
```bash
python3 ingest.py
python3 lsclient.py     # env: MINFREQ (default 3), MINLIP (10), NDATASETS (0=all), URL
python3 evaluate.py
```

## Notes
- Quantities are normalized to **per-sample relative composition before aggregation** —
  units differ across datasets and LipidSpace folds quantity into the Hausdorff.
- Generated data (`corpus.json`, `lsresult.json`) is intentionally not committed;
  regenerate with the steps above.
- The frame here is the 2D PCA the REST currently exposes; the design's dimensionality
  sweep needs the REST `PcaDimensions` parameter (see §10 caveat in the design doc).
