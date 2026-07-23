import socket
import struct
import sys

TELEM = ">IIi"
LEN_PREFIX = ">I"


def recv_exact(conn, n):
    buf = bytearray()
    while len(buf) < n:
        chunk = conn.recv(n - len(buf))
        if not chunk:
            return None
        buf.extend(chunk)
    return bytes(buf)


def main():
    port = int(sys.argv[1]) if len(sys.argv) > 1 else 9100

    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind(("127.0.0.1", port))
    server.listen(1)
    print(f"listening on :{port}", file=sys.stderr)

    conn, addr = server.accept()  # blocks until a client connects
    print(f"connection from {addr}", file=sys.stderr)

    count = 0
    with conn:
        while True:
            header = recv_exact(conn, 4)  # the length prefix
            if header is None:
                print("client closed (EOF)", file=sys.stderr)
                break
            (length,) = struct.unpack(LEN_PREFIX, header)

            payload = recv_exact(conn, length)  # exactly `length` bytes
            if payload is None:
                print("client closed mid-frame", file=sys.stderr)
                break

            device_id, seq, temp = struct.unpack(TELEM, payload)
            print(f"frame {count}: id={device_id} seq={seq} temp={temp}")
            count += 1

    print(f"received {count} frames", file=sys.stderr)
    server.close()


if __name__ == "__main__":
    main()
