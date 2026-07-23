import csv
import sys


def load(path: str) -> list[tuple[int, int, int]]:
    with open(path) as f:
        r = csv.DictReader(f)
        return [(int(x["seq"]), int(x["send_ns"]), int(x["recv_ns"])) for x in r]


def percentile(sorted_vals: list[float], p: float) -> float:
    if not sorted_vals:
        return 0.0
    return sorted_vals[min(len(sorted_vals) - 1, int(p / 100.0 * len(sorted_vals)))]


def analyze(path: str) -> None:
    rows = load(path)
    if not rows:
        print(f"=== {path} ===  (no data)\n")
        return

    seqs = [s for s, _, _ in rows]
    received = len(rows)
    expected = max(seqs) + 1
    lost = expected - received
    loss_pct = 100.0 * lost / expected
    dups = received - len(set(seqs))

    lat_us = sorted((recv - send) / 1000.0 for _, send, recv in rows)
    recv_ns = [r for _, _, r in rows]
    span_s = (max(recv_ns) - min(recv_ns)) / 1e9 if received > 1 else 0.0
    thr = received / span_s if span_s > 0 else 0.0

    print(f"=== {path} ===")
    print(
        f"  expected {expected:>8}  received {received:>8}  lost {lost:>6}  loss {loss_pct:5.2f}%  dups {dups}"
    )
    print(f"  throughput {thr:,.0f} msg/s over {span_s:.3f}s")
    print(
        f"  latency us:  p50 {percentile(lat_us, 50):8.1f}  p95 {percentile(lat_us, 95):8.1f}  p99 {percentile(lat_us, 99):8.1f}  max {lat_us[-1]:8.1f}"
    )
    print()


if __name__ == "__main__":
    paths = sys.argv[1:]
    if not paths:
        print("usage: analyze.py results_*.csv", file=sys.stderr)
        sys.exit(1)
    for p in paths:
        analyze(p)
