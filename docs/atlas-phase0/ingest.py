"""Phase 0 — ingest the mzTab-M corpus into per-dataset COMPOSITIONAL lipid profiles.

Units differ across (and within) datasets, so we normalize each sample (assay) to a
relative composition (sum to 1) BEFORE aggregating, then average the relative values
across the file's assays. That makes datasets comparable regardless of unit.

Per dataset (= one mzTab-M file) we store:
  - lipids: {chemical_name -> mean relative abundance across assays}  (sums ~1)
  - species, tissue: dataset-level label (most common sample CV term, lower-cased)
  - n_assays, n_batches (distinct ms_run refs), n_lipids
Writes corpus.json + a summary.
"""
import json, glob, os, collections

STUDIES = "/Users/nilshoffmann/Projects/github.com/lifs-tools/lipidcompass-studies/studies"
OUT = os.path.join(os.path.dirname(__file__), "corpus.json")


def parse_cv_name(value):
    v = value.strip()
    if v.startswith("[") and v.endswith("]"):
        v = v[1:-1]
    parts = [p.strip() for p in v.split(",")]
    return parts[2] if len(parts) >= 3 and parts[2] else ""


def parse_mztab(path):
    species, tissue = collections.Counter(), collections.Counter()
    assay_msrun = {}
    chem_idx = None
    assay_cols = []
    lipid_assay = collections.defaultdict(dict)   # name -> {col_i: summed value}

    with open(path, encoding="utf-8", errors="replace") as fh:
        for line in fh:
            if not line.strip():
                continue
            row = line.rstrip("\n").split("\t")
            lt = row[0]
            if lt == "MTD" and len(row) >= 3:
                key, val = row[1], row[2]
                if "-species[" in key:
                    n = parse_cv_name(val)
                    if n:
                        species[n] += 1
                elif "-tissue[" in key:
                    n = parse_cv_name(val)
                    if n:
                        tissue[n] += 1
                elif key.endswith("-ms_run_ref"):
                    assay_msrun[key.split("-")[0]] = val
            elif lt == "SMH":
                for i, c in enumerate(row):
                    if c == "chemical_name":
                        chem_idx = i
                    elif c.startswith("abundance_assay["):
                        assay_cols.append(i)
            elif lt == "SML" and chem_idx is not None:
                name = row[chem_idx].strip() if chem_idx < len(row) else ""
                if not name:
                    continue
                for i in assay_cols:
                    if i < len(row):
                        s = row[i].strip()
                        if s and s.lower() not in ("null", "nan", "na", "inf"):
                            try:
                                v = float(s)
                            except ValueError:
                                continue
                            if v > 0:
                                lipid_assay[name][i] = lipid_assay[name].get(i, 0.0) + v

    # per-assay totals, then mean relative abundance per lipid over valid assays
    assay_total = collections.defaultdict(float)
    for d in lipid_assay.values():
        for i, v in d.items():
            assay_total[i] += v
    valid = [i for i in assay_cols if assay_total[i] > 0]
    na = len(valid)
    rel = {}
    if na:
        for name, d in lipid_assay.items():
            s = sum(d[i] / assay_total[i] for i in valid if i in d)
            if s > 0:
                rel[name] = s / na

    return {
        "lipids": rel,
        "species": (species.most_common(1)[0][0] if species else "unknown").strip().lower(),
        "tissue": (tissue.most_common(1)[0][0] if tissue else "unknown").strip().lower(),
        "n_assays": na,
        "n_batches": len(set(assay_msrun.values())) if assay_msrun else 0,
        "n_lipids": len(rel),
    }


def main():
    files = sorted(glob.glob(os.path.join(STUDIES, "*", "data", "*.mzTab")))
    corpus, failed = {}, []
    for path in files:
        ds_id = os.path.basename(path)[:-len(".mzTab")]
        try:
            rec = parse_mztab(path)
            if rec["n_lipids"] >= 3:
                corpus[ds_id] = rec
            else:
                failed.append((ds_id, f"{rec['n_lipids']} lipids"))
        except Exception as e:
            failed.append((ds_id, repr(e)))

    json.dump(corpus, open(OUT, "w"))

    n = len(corpus)
    print(f"Parsed {n} datasets ({len(failed)} skipped) from {len(files)} files")
    import statistics as st
    sp = collections.Counter(d["species"] for d in corpus.values())
    ti = collections.Counter(d["tissue"] for d in corpus.values())
    print("species:", dict(sp.most_common(6)))
    print("tissue :", dict(ti.most_common(8)))
    nl = [d["n_lipids"] for d in corpus.values()]
    print(f"lipids/dataset median {int(st.median(nl))} (min {min(nl)}, max {max(nl)})")
    # sanity: relative profiles should sum ~1
    sums = [sum(d["lipids"].values()) for d in corpus.values()]
    print(f"profile sums: min {min(sums):.3f} max {max(sums):.3f} (expect ~1)")


if __name__ == "__main__":
    main()
