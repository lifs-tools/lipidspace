"""Phase 0 — call LipidSpace /pca to get the frozen frame + Hausdorff ground truth.

Sends the corpus (one relative-composition lipidome per dataset) as a COLUMN_PIVOT_TABLE.
Quantities are already per-sample relative (see ingest.py) so they are unit-compatible —
important because LipidSpace folds the quantity into the Hausdorff comparison.

Parses back and saves lsresult.pkl:
  - frame_xy: [[x,y], ...] for all frame lipids (module k-means basis)
  - order:    dataset ids aligned to the Hausdorff matrix rows
  - clouds:   {ds -> [[x,y,w], ...]} its point cloud with relative-abundance weights
  - hausdorff: NxN LipidSpace distance matrix (structure + folded quantity)
  - meta:     {ds -> {species, tissue}}

Env: MINFREQ (min #datasets a lipid appears in; default 3), MINLIP (min frame lipids
per dataset; default 10), NDATASETS (cap for testing; 0=all), URL.
"""
import json, os, time, pickle, collections
import requests

HERE = os.path.dirname(__file__)
URL = os.environ.get("URL", "http://localhost:28100/lipidspace/v1/pca")
MINFREQ = int(os.environ.get("MINFREQ", "3"))
MINLIP = int(os.environ.get("MINLIP", "10"))
NDATASETS = int(os.environ.get("NDATASETS", "0"))
PCADIMS = int(os.environ.get("PCADIMS", "2"))


def qcsv_row(fields):
    return ",".join('"' + str(f).replace('"', '""') + '"' for f in fields)


def build_payload(corpus, ds_ids, lipids):
    rows = [qcsv_row(["Sample"] + lipids)]
    for ds in ds_ids:
        lp = corpus[ds]["lipids"]
        rows.append(qcsv_row([ds] + [lp.get(l, "") for l in lipids]))
    return {
        "TableType": "COLUMN_PIVOT_TABLE",
        "TableColumnTypes": ["SampleColumn"] + ["LipidColumn"] * len(lipids),
        "Table": "\n".join(rows),
        "PcaDimensions": PCADIMS,
    }


def coords_from(entry):
    """D-dim coords per lipid: PcaCoordinates (column-major) if present, else X/Y."""
    pc = entry.get("PcaCoordinates")
    if pc:
        ndim, npts = len(pc), len(pc[0])
        return [[pc[k][i] for k in range(ndim)] for i in range(npts)]
    return [[x, y] for x, y in zip(entry["X"], entry["Y"])]


def is_sample(name):
    return name != "global_lipidome" and "group lipidome" not in name


def clean(name):  # LipidSpace appends the auto 'Origin' study variable to sample names
    suf = " - table_file"
    return name[:-len(suf)] if name.endswith(suf) else name


def main():
    corpus = json.load(open(os.path.join(HERE, "corpus.json")))
    ds_ids = sorted(corpus)
    if NDATASETS:
        ds_ids = ds_ids[:NDATASETS]

    freq = collections.Counter()
    for ds in ds_ids:
        for l in corpus[ds]["lipids"]:
            freq[l] += 1
    lipids = sorted(l for l, c in freq.items() if c >= MINFREQ)

    lipset = set(lipids)
    ds_ids = [d for d in ds_ids if len(set(corpus[d]["lipids"]) & lipset) >= MINLIP]
    print(f"start: datasets={len(ds_ids)}  lipids(freq>={MINFREQ})={len(lipids)}")

    glob = samples = H = None
    for it in range(6):
        payload = build_payload(corpus, ds_ids, lipids)
        mb = len(payload["Table"]) / 1e6
        t0 = time.time()
        r = requests.post(URL, json=payload, timeout=1800)
        dt = time.time() - t0
        print(f"[iter {it}] datasets={len(ds_ids)} table={mb:.1f}MB "
              f"-> HTTP {r.status_code} in {dt:.1f}s")
        if not r.ok:
            print(r.text[:400]); return
        resp = r.json()
        spaces = resp["LipidSpaces"]
        glob = next(s for s in spaces if s["LipidomeName"] == "global_lipidome")
        samples = [s for s in spaces if is_sample(s["LipidomeName"])]
        H = resp["LipidomeDistanceMatrix"]
        if len(samples) == len(H):
            break
        k = len(samples) - len(H)
        idx = sorted(range(len(samples)),
                     key=lambda i: len(samples[i]["LipidNames"]))[:k]
        dropset = {clean(samples[i]["LipidomeName"]) for i in idx}
        print(f"  LipidSpace excluded {k}; dropping sparse datasets: {sorted(dropset)}")
        ds_ids = [d for d in ds_ids if d not in dropset]
    assert len(samples) == len(H), f"unresolved: {len(samples)} != {len(H)}"

    order = [clean(s["LipidomeName"]) for s in samples]
    frame_xy = coords_from(glob)
    clouds = {clean(s["LipidomeName"]):
              [c + [w] for c, w in zip(coords_from(s), s["Intensities"])]
              for s in samples}
    meta = {ds: {"species": corpus[ds]["species"], "tissue": corpus[ds]["tissue"]}
            for ds in order}
    print(f"FINAL frame lipids={len(frame_xy)}  dims={len(frame_xy[0])}  "
          f"datasets={len(order)}  H={len(H)}x{len(H[0])}")

    with open(os.path.join(HERE, "lsresult.json"), "w") as fh:
        json.dump({"frame_xy": frame_xy, "order": order, "clouds": clouds,
                   "hausdorff": H, "meta": meta,
                   "minfreq": MINFREQ, "minlip": MINLIP, "dims": len(frame_xy[0])}, fh)
    print("saved lsresult.json")


if __name__ == "__main__":
    main()
