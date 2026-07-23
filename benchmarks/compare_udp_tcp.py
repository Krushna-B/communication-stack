#!/usr/bin/env python3
# /// script
# dependencies = ["matplotlib"]
# ///
# compare_udp_tcp.py [LABEL:path.csv ...]
# Default compares UDP max-blast vs TCP max-blast (same seq/send_ns/recv_ns schema).

import csv
import sys

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt

DEFAULTS = [
    "UDP:../01-upd/experiment/results_unlimited.csv",
    "TCP:../02-tcp/experiment/results_d0.csv",
]


def load(path: str) -> list[tuple[int, int, int]]:
    with open(path) as f:
        r = csv.DictReader(f)
        return [(int(x["seq"]), int(x["send_ns"]), int(x["recv_ns"])) for x in r]


def percentile(v: list[float], p: float) -> float:
    return v[min(len(v) - 1, int(p / 100.0 * len(v)))] if v else 0.0


def summarize(label: str, path: str) -> dict:
    rows = load(path)
    received = len(rows)
    expected = max(s for s, _, _ in rows) + 1
    recv_ns = [r for _, _, r in rows]
    span_s = (max(recv_ns) - min(recv_ns)) / 1e9 if received > 1 else 0.0
    lat_us = sorted((recv - send) / 1000.0 for _, send, recv in rows)
    return {
        "label": label,
        "loss_pct": 100.0 * (expected - received) / expected,
        "throughput_k": (received / span_s / 1000.0) if span_s > 0 else 0.0,
        "p50": percentile(lat_us, 50),
        "p99": percentile(lat_us, 99),
    }


def main() -> None:
    args = sys.argv[1:] or DEFAULTS
    runs = [summarize(*a.split(":", 1)) for a in args]
    labels = [r["label"] for r in runs]
    colors = ["tab:red", "tab:blue", "tab:green", "tab:orange"][: len(runs)]

    fig, axes = plt.subplots(1, 3, figsize=(13, 4.5))

    axes[0].bar(labels, [r["loss_pct"] for r in runs], color=colors)
    axes[0].set_title("packet loss (%)")
    axes[0].set_ylabel("% lost")

    axes[1].bar(labels, [r["throughput_k"] for r in runs], color=colors)
    axes[1].set_title("delivered throughput (k msg/s)")

    x = range(len(runs))
    w = 0.35
    axes[2].bar([i - w / 2 for i in x], [r["p50"] for r in runs], w, label="p50")
    axes[2].bar([i + w / 2 for i in x], [r["p99"] for r in runs], w, label="p99")
    axes[2].set_yscale("log")
    axes[2].set_xticks(list(x))
    axes[2].set_xticklabels(labels)
    axes[2].set_title("latency (us, log)")
    axes[2].legend()

    fig.suptitle("UDP vs TCP under max-blast overload")
    fig.tight_layout()
    fig.savefig("compare_udp_tcp.png", dpi=130)

    for r in runs:
        print(f"{r['label']:5} loss {r['loss_pct']:5.2f}%  "
              f"thr {r['throughput_k']:7.1f}k msg/s  "
              f"p50 {r['p50']:.0f}us  p99 {r['p99']:.0f}us")
    print("wrote compare_udp_tcp.png")


if __name__ == "__main__":
    main()
