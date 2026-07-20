"""Phase-0 preview: candidate module allocation from COMP_DB per-class species counts.

k_class = clamp( round( K_total * count^beta / sum(count^beta) ), kmin, count )

This is the "complexity score as allocation" the design calls for, computed on real
COMP_DB counts. It is a preview of the ADAPTIVE-SPLIT target, not the split itself:
the real k_class emerges from splitting each class's frozen-frame points to a spread
epsilon; this shows whether the counts imply a sane, non-degenerate allocation.
"""
import csv, collections

path = "COMP_DB_DATA.tsv"
counts = collections.Counter()
with open(path) as f:
    for row in csv.DictReader(f, delimiter="\t"):
        counts[row["headgroup"]] += 1

total_species = sum(counts.values())
n_classes = len(counts)


def allocate(counts, beta, K_total, kmin=1):
    w = {c: n ** beta for c, n in counts.items()}
    S = sum(w.values())
    return {c: min(max(kmin, round(K_total * w[c] / S)), n) for c, n in counts.items()}


reps = ["TG", "DG", "PC", "PE", "PI", "PS", "PG", "PA", "ST", "PIP",
        "Cer", "SM", "HexCer", "M(IP)2C", "Hex(2)-HexNAc-NeuAc-Cer"]

print(f"COMP_DB: {total_species} species across {n_classes} classes\n")
for beta in (0.5, 0.35):
    for K_total in (300, 600):
        k = allocate(counts, beta, K_total)
        realized = sum(k.values())
        at_floor = sum(1 for c in counts if k[c] == 1)
        mx = max(k, key=k.get)
        avg_lpm = total_species / realized
        print(f"beta={beta}  target={K_total}  ->  realized K={realized}  "
              f"(avg {avg_lpm:.1f} species/module; {at_floor}/{n_classes} at floor; "
              f"richest {mx}={k[mx]})")
        for c in reps:
            if c in counts:
                bar = "#" * k[c]
                print(f"    {c:26s} {counts[c]:6d} sp -> {k[c]:3d}  {bar}")
        print()
