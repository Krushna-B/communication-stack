"""Plot the UDP sweep results"""

import csv
import glob
import math
import sys

import matplotlib

matplotlib.use("Agg")  # no display needed; save to file
import matplotlib.pyplot as plt


def rate_from_name(path: str) -> float:
    label = path.split("results_")[-1].rsplit(".csv", 1)[0]
    return math.inf if label == "unlimited" else float(label)


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
    seqs = [s for s, _, _ in rows]
    received = len(rows)
    expected = max(seqs) + 1
    loss_pct = 100.0 * (expected - received) / expected
    recv_ns = [r for _, _, r in rows]
    span_s = (max(recv_ns) - min(recv_ns)) / 1e9 if received > 1 else 0.0
    thr = received / span_s if span_s > 0 else 0.0
    lat_us = sorted((recv - send) / 1000.0 for _, send, recv in rows)
    return {
        "rate": rate_from_name(path),
        "label": path.split("results_")[-1].rsplit(".csv", 1)[0],
        "loss_pct": loss_pct,
        "throughput": thr,
        "p50": percentile(lat_us, 50),
        "p95": percentile(lat_us, 95),
        "p99": percentile(lat_us, 99),
        "lat_us": lat_us,
    }


def main() -> None:
    paths = sys.argv[1:] or sorted(glob.glob("results_*.csv"))
    if not paths:
        print("no results_*.csv found", file=sys.stderr)
        sys.exit(1)

    runs = sorted((summarize(p) for p in paths), key=lambda d: d["rate"])
    x = list(range(len(runs)))
    labels = [r["label"] for r in runs]

    # --- Plot 1: loss % and achieved throughput vs requested rate ---
    fig, ax1 = plt.subplots(figsize=(8, 5))
    ax1.plot(
        x,
        [r["throughput"] / 1000 for r in runs],
        "o-",
        color="tab:blue",
        label="achieved throughput",
    )
    ax1.set_xlabel("requested send rate")
    ax1.set_ylabel("achieved throughput (k pps)", color="tab:blue")
    ax1.tick_params(axis="y", labelcolor="tab:blue")
    ax1.set_xticks(x)
    ax1.set_xticklabels(labels, rotation=30)

    ax2 = ax1.twinx()
    ax2.plot(x, [r["loss_pct"] for r in runs], "s--", color="tab:red", label="loss %")
    ax2.set_ylabel("packet loss (%)", color="tab:red")
    ax2.tick_params(axis="y", labelcolor="tab:red")

    plt.title("UDP: throughput plateaus and loss climbs past the cliff")
    fig.tight_layout()
    fig.savefig("plot_loss_throughput.png", dpi=130)

    # --- Plot 2: latency percentiles vs rate ---
    fig, ax = plt.subplots(figsize=(8, 5))
    for key, style in (("p50", "o-"), ("p95", "s-"), ("p99", "^-")):
        ax.plot(x, [r[key] for r in runs], style, label=key)
    ax.set_yscale("log")
    ax.set_xlabel("requested send rate")
    ax.set_ylabel("one-way latency (us, log scale)")
    ax.set_xticks(x)
    ax.set_xticklabels(labels, rotation=30)
    ax.legend()
    ax.set_title("Latency tail balloons as the receive queue fills")
    fig.tight_layout()
    fig.savefig("plot_latency.png", dpi=130)

    # --- Plot 3: latency CDF for a few representative runs ---
    fig, ax = plt.subplots(figsize=(8, 5))
    pick = {"100000", "200000", "unlimited"}
    for r in runs:
        if r["label"] not in pick:
            continue
        lat = r["lat_us"]
        ys = [i / len(lat) for i in range(len(lat))]
        ax.plot(lat, ys, label=r["label"])
    ax.set_xscale("log")
    ax.set_xlabel("one-way latency (us, log scale)")
    ax.set_ylabel("cumulative fraction")
    ax.legend()
    ax.set_title("Latency CDF: below vs above the cliff")
    fig.tight_layout()
    fig.savefig("plot_cdf.png", dpi=130)

    print("wrote plot_loss_throughput.png, plot_latency.png, plot_cdf.png")


if __name__ == "__main__":
    main()
