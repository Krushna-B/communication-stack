import csv
import socket
import struct
import sys
import time

HDR = ">IQ"
HDR_SIZE = struct.calcsize(HDR)
LEN_PREFIX = ">I"


def recv_exact(conn: socket.socket, n: int) -> bytes | None:
    buf = bytearray()
    while len(buf) < n:
        chunk = conn.recv(n - len(buf))
        if not chunk:
            return None
        buf.extend(chunk)
    return bytes(buf)


def busy_wait_us(us: float) -> None:
    if us <= 0:
        return
    end = time.perf_counter_ns() + int(us * 1000)
    while time.perf_counter_ns() < end:
        pass


def main() -> None:
    port = int(sys.argv[1]) if len(sys.argv) > 1 else 9100
    delay_us = float(sys.argv[2]) if len(sys.argv) > 2 else 0.0
    out_csv = sys.argv[3] if len(sys.argv) > 3 else "results.csv"

    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind(("127.0.0.1", port))
    server.listen(1)
    print(f"listening :{port}  delay={delay_us}us  -> {out_csv}", file=sys.stderr)

    conn, _ = server.accept()
    rows: list[tuple[int, int, int]] = []
    with conn:
        while True:
            header = recv_exact(conn, 4)
            if header is None:
                break
            (length,) = struct.unpack(LEN_PREFIX, header)
            payload = recv_exact(conn, length)
            if payload is None:
                break
            recv_ns = time.clock_gettime_ns(time.CLOCK_MONOTONIC)
            seq, send_ns = struct.unpack(HDR, payload[:HDR_SIZE])
            rows.append((seq, send_ns, recv_ns))
            busy_wait_us(delay_us)
    server.close()

    with open(out_csv, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["seq", "send_ns", "recv_ns"])
        w.writerows(rows)
    print(f"wrote {len(rows)} rows to {out_csv}", file=sys.stderr)


if __name__ == "__main__":
    main()
