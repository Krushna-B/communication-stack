#include "telemetry.hpp"
#include <cstdint>
#include <fstream>
#include <iostream>
#include <stdexcept>

int main() {
  const uint32_t device_id = 55;
  const uint32_t sequence_number = 2;
  const int32_t temp = 87;

  std::array<std::byte, MESSAGE_SIZE> buffer{};

  write_uint32(buffer, 0, device_id);
  write_uint32(buffer, 4, sequence_number);
  write_uint32(buffer, 8, static_cast<std::uint32_t>(temp));
  print_buffer(buffer);

  // Stream buffer to binary file
  std::ofstream out("telemetry.bin", std::ios::binary);
  out.write(reinterpret_cast<const char *>(buffer.data()), buffer.size());

  const std::uint32_t decoded_device_id = read_uint32(buffer, 0);

  const std::uint32_t decoded_sequence = read_uint32(buffer, 4);

  const std::int32_t decoded_temperature =
      static_cast<std::int32_t>(read_uint32(buffer, 8));

  if (decoded_device_id != device_id || decoded_sequence != sequence_number ||
      decoded_temperature != temp) {
    throw std::runtime_error("Decoded values do not match");
  }

  std::cout << "device_id: " << decoded_device_id << '\n';

  std::cout << "sequence_number: " << decoded_sequence << '\n';

  std::cout << "temperature: " << decoded_temperature << " C\n";
}
