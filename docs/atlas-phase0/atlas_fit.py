"""Demonstrate + validate the Atlas: leave-one-dataset-out fit (retrieval + confidence-gated
accuracy + OOD), example fits, and an incremental-add timing check."""
import json, os, time, collections
import numpy as np
from atlas import Atlas, fingerprint_lipidome

HERE = os.path.dirname(__file__)
atlas = Atlas.load(os.path.join(HERE, "atlas.json"))
queries = json.load(open(os.path.join(HERE, "demo_queries.json")))
sup = collections.Counter(m["tissue"] for m in atlas.meta.values())

# ---- example fits (a common tissue, a distinctive one, and a rare/OOD one) ----
examples = []
for want in ("plasma", "brain", "liver"):
    for ds in atlas.datasets:
        if atlas.meta[ds]["tissue"] == want and sup[want] >= 5:
            examples.append(ds); break
rare = [ds for ds in atlas.datasets if sup[atlas.meta[ds]["tissue"]] <= 3]
if rare:
    examples.append(rare[0])

print("=== example fits (leave-one-out) ===")
for ds in examples:
    r = atlas.fit(queries[ds], k=5, exclude=ds)
    truth = atlas.meta[ds]["tissue"]
    print(f"\nquery {ds}  (true tissue: {truth}, {atlas.meta[ds]['species']})")
    print(f"  predicted: {r['pred_tissue']}  vote={r['vote']}  confidence={r['confidence']}  "
          f"ood={r['ood']}  coverage={r['coverage']}  nn_dist={r['nn_dist']}")
    for nb, d, t in r["neighbors"]:
        print(f"    {nb:14s} d={d:.3f}  {t}")

# ---- LODO aggregate: retrieval + confidence gating + OOD ----
preds, truths, confs, oods, correct = [], [], [], [], []
for ds in atlas.datasets:
    r = atlas.fit(queries[ds], k=5, exclude=ds)
    preds.append(r["pred_tissue"]); truths.append(atlas.meta[ds]["tissue"])
    confs.append(r["confidence"]); oods.append(r["ood"])
    correct.append(r["pred_tissue"] == atlas.meta[ds]["tissue"])
correct = np.array(correct); confs = np.array(confs)
n = len(correct)
print("\n=== LODO aggregate over the atlas ===")
print(f"  overall tissue accuracy: {correct.mean():.3f}")
for cov in (0.5, 0.3, 0.15):
    idx = np.argsort(-confs)[:max(1, round(cov * n))]
    print(f"  accuracy @ top-{int(cov*100)}% confidence: {correct[idx].mean():.3f}")
minority = np.array([sup[t] <= 3 for t in truths])
print(f"  mean confidence: populated tissues={confs[~minority].mean():.3f}  "
      f"rare tissues={confs[minority].mean():.3f}")
print(f"  flagged OOD: {sum(oods)}/{n}  (rare-tissue flagged {sum(o for o, m in zip(oods, minority) if m)}"
      f"/{minority.sum()})")

# ---- incremental add: O(size), no rebuild ----
ds0 = atlas.datasets[0]
t0 = time.time()
for _ in range(50):
    atlas.add_dataset("NEW", dict(queries[ds0]), {"tissue": "x", "species": "x", "study": "x"})
    atlas.datasets.pop(); atlas.F = atlas.F[:-1]; atlas.meta.pop("NEW")
per = (time.time() - t0) / 50 * 1000
print(f"\n=== incremental add ===")
print(f"  fingerprint + append one dataset: {per:.2f} ms  (no frame/module/other recompute)")
