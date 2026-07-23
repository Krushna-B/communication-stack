#include "telemetry.hpp"
#include <array>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <stdexcept>

int main() {
  const uint32_t device_id = 55;
  const uint32_t sequence_number = 2;
  const int32_t temp = 87;

  std::array<std::byte, MESSAGE_SIZE> buffer{};

  telemetry::write(buffer, 0, device_id);
  telemetry::write(buffer, 4, sequence_number);
  telemetry::write(buffer, 8, temp);
  print_buffer(buffer);

  // Stream buffer to binary file
  std::ofstream out("telemetry.bin", std::ios::binary);
  out.write(reinterpret_cast<const char *>(buffer.data()), buffer.size());

  const std::uint32_t decoded_device_id = telemetry::read<std::uint32_t>(buffer, 0);

  const std::uint32_t decoded_sequence = telemetry::read<std::uint32_t>(buffer, 4);

  const std::int32_t decoded_temperature = telemetry::read<std::int32_t>(buffer, 8);

  if (decoded_device_id != device_id || decoded_sequence != sequence_number ||
      decoded_temperature != temp) {
    throw std::runtime_error("Decoded values do not match");
  }

  std::cout << "device_id: " << decoded_device_id << '\n';

  std::cout << "sequence_number: " << decoded_sequence << '\n';

  std::cout << "temperature: " << decoded_temperature << " C\n";
}
