import socket
import struct

HOST, PORT = "127.0.0.1", 9000
FORMAT = ">IIi"

##Create socket and bind to addr:port
with socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM) as sock:
    sock.bind((HOST, PORT))
    print(f"Socket listenting on {HOST}:{PORT}")

    while True:
        data, addr = sock.recvfrom(1024)
        if len(data) != 12:
            print(f"expected 12 bytes, got {len(data)}")
            continue
        device_id, seq, temp = struct.unpack(FORMAT, data)
        print(f"from {addr}: id={device_id} seq={seq} temp={temp}")
