import csv
import glob
import sys

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt


def delay_from_name(path: str) -> float:
    label = path.split("results_d")[-1].rsplit(".csv", 1)[0]
    return float(label)


def load(path: str) -> list[tuple[int, int, int]]:
    with open(path) as f:
        r = csv.DictReader(f)
        return [(int(x["seq"]), int(x["send_ns"]), int(x["recv_ns"])) for x in r]


def percentile(sorted_vals: list[float], p: float) -> float:
    if not sorted_vals:
        return 0.0
    return sorted_vals[min(len(sorted_vals) - 1, int(p / 100.0 * len(sorted_vals)))]


def summarize(path: str) -> dict:
    rows = load(path)
    received = len(rows)
    recv_ns = [r for _, _, r in rows]
    span_s = (max(recv_ns) - min(recv_ns)) / 1e9 if received > 1 else 0.0
    lat_us = sorted((recv - send) / 1000.0 for _, send, recv in rows)
    return {
        "delay": delay_from_name(path),
        "throughput": received / span_s if span_s > 0 else 0.0,
        "p50": percentile(lat_us, 50),
        "p95": percentile(lat_us, 95),
        "p99": percentile(lat_us, 99),
    }


def main() -> None:
    paths = sys.argv[1:] or sorted(glob.glob("results_d*.csv"))
    if not paths:
        print("no results_d*.csv found", file=sys.stderr)
        sys.exit(1)

    runs = sorted((summarize(p) for p in paths), key=lambda d: d["delay"])
    delays = [r["delay"] for r in runs]

    fig, ax = plt.subplots(figsize=(8, 5))
    ax.plot(delays, [r["throughput"] / 1000 for r in runs], "o-", color="tab:blue")
    ax.set_xlabel("receiver delay per message (us)")
    ax.set_ylabel("achieved throughput (k msg/s)")
    ax.set_title("TCP: throughput is bounded by the receiver (no loss)")
    fig.tight_layout()
    fig.savefig("plot_throughput.png", dpi=130)

    fig, ax = plt.subplots(figsize=(8, 5))
    for key, style in (("p50", "o-"), ("p95", "s-"), ("p99", "^-")):
        ax.plot(delays, [r[key] for r in runs], style, label=key)
    ax.set_yscale("log")
    ax.set_xlabel("receiver delay per message (us)")
    ax.set_ylabel("one-way latency (us, log scale)")
    ax.legend()
    ax.set_title("TCP: latency grows under backpressure, still zero loss")
    fig.tight_layout()
    fig.savefig("plot_latency.png", dpi=130)

    print("wrote plot_throughput.png, plot_latency.png")


if __name__ == "__main__":
    main()
