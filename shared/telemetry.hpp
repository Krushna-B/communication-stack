#pragma once

#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <span>
#include <type_traits>

constexpr std::size_t MESSAGE_SIZE = 12;

namespace telemetry {

/**
Utilizing span just points to values in the buffer
 */

/**
@brief Writes an integer into buffer at offset in big-endian order.

@param buffer non-owning view of the raw-byte buffer
@param offset byte position where this field starts
@param value integer value to encode
 */
template <typename T>
inline void write(std::span<std::byte> buffer, std::size_t offset, T value) {
  static_assert(std::is_integral_v<T>, "write requires an integer type");
  for (std::size_t i{0}; i < sizeof(value); i++) {
    buffer[offset + i] =
        static_cast<std::byte>(value >> ((sizeof(value) - 1 - i) * 8) & 0xFF);
  }
}

/**
@brief Reads a big-endian integer of type T from buffer at offset.

@param buffer non-owning view of the raw-byte buffer
@param offset byte position where this field starts
@returns the decoded value
 */
template <typename T>
inline T read(std::span<const std::byte> buffer, std::size_t offset) {
  static_assert(std::is_integral_v<T>, "read requires an integer type");
  T output{};
  for (std::size_t i{0}; i < sizeof(T); i++) {
    auto val = std::to_integer<T>(buffer[offset + i]);
    output |= val << ((sizeof(T) - 1 - i) * 8);
  }
  return output;
}

} // namespace telemetry

/**
@brief Prints every byte of the buffer as space-separated hex on one line.
 */
inline void print_buffer(std::span<const std::byte> buffer) {
  for (std::byte b : buffer) {
    std::cout << std::hex << std::setw(2) << std::setfill('0')
              << std::to_integer<int>(b) << ' ';
  }
  std::cout << std::dec << '\n';
}
