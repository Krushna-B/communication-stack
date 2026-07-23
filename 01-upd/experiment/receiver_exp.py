"""UDP experiment receiver: logs (seq, send_ns, recv_ns) per datagram to CSV.

Usage: receiver_exp.py [port=9000] [rcvbuf_bytes=0] [out_csv=results.csv]

"""

import csv
import socket
import struct
import sys
import time

HDR = ">IQ"  # uint32 sequence_number, uint64 send_ns (big-endian)
HDR_SIZE = struct.calcsize(HDR)  # 12
SENTINEL = 0xFFFFFFFF


def main() -> None:
    port = int(sys.argv[1]) if len(sys.argv) > 1 else 9000
    rcvbuf = int(sys.argv[2]) if len(sys.argv) > 2 else 0
    out_csv = sys.argv[3] if len(sys.argv) > 3 else "results.csv"

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    if rcvbuf > 0:
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, rcvbuf)

    actual = sock.getsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF)
    sock.bind(("127.0.0.1", port))
    sock.settimeout(3.0)  # stop if idle 3s
    print(f"listening :{port}  SO_RCVBUF={actual}  -> {out_csv}", file=sys.stderr)

    rows: list[tuple[int, int, int]] = []
    while True:
        try:
            data, _ = sock.recvfrom(65535)
        except socket.timeout:
            print("idle timeout, stopping", file=sys.stderr)
            break
        recv_ns = time.clock_gettime_ns(time.CLOCK_MONOTONIC)
        if len(data) < HDR_SIZE:
            continue
        seq, send_ns = struct.unpack(HDR, data[:HDR_SIZE])
        if seq == SENTINEL:
            print("got sentinel, stopping", file=sys.stderr)
            break
        rows.append((seq, send_ns, recv_ns))

    sock.close()

    with open(out_csv, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["seq", "send_ns", "recv_ns"])
        w.writerows(rows)
    print(f"wrote {len(rows)} rows to {out_csv}", file=sys.stderr)


if __name__ == "__main__":
    main()
