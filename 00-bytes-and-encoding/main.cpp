#include <array>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>

constexpr size_t MESSAGE_SIZE = 12;

// Function headers
void write_uint32(std::array<std::byte, MESSAGE_SIZE> &buffer, size_t offset,
                  std::uint32_t value);
uint32_t read_uint32(std::array<std::byte, MESSAGE_SIZE> &buffer,
                     size_t offset);
void print_buffer(const std::array<std::byte, MESSAGE_SIZE> &buffer);

/**
@brief Writes uint32_t integers to buffer

@param buffer is reference to buffere holding raw bytes
@param offset is
@param value is uint32_t integer value you want to write to buffer
 */
void write_uint32(std::array<std::byte, MESSAGE_SIZE> &buffer, size_t offset,
                  std::uint32_t value) {

  for (size_t i{0}; i < sizeof(value); i++) {
    // Shift down every byte, this is little endian value
    buffer[offset + i] =
        static_cast<std::byte>(value >> ((sizeof(value) - 1 - i) * 8) & 0xFF);
  }
  return;
}

/**
@brief Reads uint32_t integers from buffer

@param buffer is reference to buffere holding raw bytes
@param offset is

@returns uint32_t integer from buffer
 */
uint32_t read_uint32(std::array<std::byte, MESSAGE_SIZE> &buffer,
                     size_t offset) {
  uint32_t output{};
  for (size_t i{}; i < sizeof(uint32_t); i++) {
    auto val = (std::to_integer<uint32_t>(buffer[offset + i]));
    output |= val << ((sizeof(val) - 1 - i) * 8);
  }
  return output;
}

void print_buffer(const std::array<std::byte, MESSAGE_SIZE> &buffer) {
  for (std::byte b : buffer) {
    std::cout << std::hex << std::setw(2) << std::setfill('0')
              << std::to_integer<int>(b) << ' ';
    std::cout << std::dec << '\n';
  }
}

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
