#pragma once

#include "telemetry.hpp"
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <span>

namespace exp {

constexpr std::size_t HDR_SIZE = 12;
struct Packet {
  std::uint32_t sequence_number{};
  std::uint64_t send_ns{};
};

/**
@brief Monotonic clock in nanoseconds
 */
inline std::uint64_t ns_now() {
  const auto now = std::chrono::steady_clock::now();
  const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                      now.time_since_epoch())
                      .count();
  return static_cast<std::uint64_t>(ns);
}

/**Serialize pkt into the first header of the buffer
 */
inline void encode(std::span<std::byte> buffer, const Packet &pkt) {
  telemetry::write(buffer, 0, pkt.sequence_number);
  telemetry::write(buffer, 4, pkt.send_ns);
}

inline Packet decode(std::span<const std::byte> buffer) {
  Packet pkt{};
  pkt.sequence_number = telemetry::read<std::uint32_t>(buffer, 0);
  pkt.send_ns = telemetry::read<std::uint64_t>(buffer, 4);
  return pkt;
}

} // namespace exp
