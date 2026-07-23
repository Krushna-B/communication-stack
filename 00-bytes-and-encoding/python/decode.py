import struct
import sys

MESSAGE_SIZE = 12
# ">" = big-endian (network byte order)

# device_id: uint32, sequence_number: uint32, temperature_milli: int32.
FORMAT = ">IIi"

assert struct.calcsize(FORMAT) == MESSAGE_SIZE


def decode(data: bytes):
    if len(data) != MESSAGE_SIZE:
        raise ValueError(f"expected {MESSAGE_SIZE} bytes, got {len(data)}")
    device_id, sequence_number, temperature_milli = struct.unpack(FORMAT, data)
    return device_id, sequence_number, temperature_milli


def main(path: str):
    # Open with rb is reading as binary
    with open(path, "rb") as f:
        data = f.read()

    device_id, sequence_number, temperature_milli = decode(data)

    print(f"device_id:          {device_id}")
    print(f"sequence_number:    {sequence_number}")
    print(
        f"temperature_milli:  {temperature_milli}  ({temperature_milli / 1000:.3f} C)"
    )


if __name__ == "__main__":
    main(sys.argv[1] if len(sys.argv) > 1 else "telemetry.bin")
