#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>

constexpr std::size_t MESSAGE_SIZE = 12;

/**
@brief Writes a uint32_t into buffer at offset in big-endian order.

@param buffer reference to the raw-byte buffer
@param offset byte position where this field starts
@param value uint32_t value to encode
 */
inline void write_uint32(std::array<std::byte, MESSAGE_SIZE> &buffer,
                         std::size_t offset, std::uint32_t value) {
  for (std::size_t i{0}; i < sizeof(value); i++) {
    buffer[offset + i] =
        static_cast<std::byte>(value >> ((sizeof(value) - 1 - i) * 8) & 0xFF);
  }
}

/**
@brief Reads a big-endian uint32_t from buffer at offset.

@param buffer reference to the raw-byte buffer
@param offset byte position where this field starts
@returns the decoded uint32_t
 */
inline std::uint32_t
read_uint32(const std::array<std::byte, MESSAGE_SIZE> &buffer,
            std::size_t offset) {
  std::uint32_t output{};
  for (std::size_t i{0}; i < sizeof(std::uint32_t); i++) {
    auto val = std::to_integer<std::uint32_t>(buffer[offset + i]);
    output |= val << ((sizeof(val) - 1 - i) * 8);
  }
  return output;
}

/**
@brief Prints every byte of the buffer as space-separated hex on one line.
 */
inline void print_buffer(const std::array<std::byte, MESSAGE_SIZE> &buffer) {
  for (std::byte b : buffer) {
    std::cout << std::hex << std::setw(2) << std::setfill('0')
              << std::to_integer<int>(b) << ' ';
  }
  std::cout << std::dec << '\n';
}
