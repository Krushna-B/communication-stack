// UDP telemetry blast for the loss/latency/throughput experiment.
//
//   ./sender_exp <count> [rate_pps=0] [payload=20] [host=127.0.0.1] [port=9000]
//

#include "exp_packet.hpp"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <thread>
#include <vector>

// Wire timestamp
static std::uint64_t wire_now_ns() {
  timespec ts{};
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return static_cast<std::uint64_t>(ts.tv_sec) * 1'000'000'000ULL +
         static_cast<std::uint64_t>(ts.tv_nsec);
}

int main(int argc, char **argv) {
  const std::uint32_t count =
      argc > 1 ? static_cast<std::uint32_t>(std::strtoul(argv[1], nullptr, 10))
               : 100000;
  const double rate = argc > 2 ? std::strtod(argv[2], nullptr) : 0.0; // pps
  const std::size_t payload =
      argc > 3 ? static_cast<std::size_t>(std::strtoul(argv[3], nullptr, 10))
               : 20;
  const char *host = argc > 4 ? argv[4] : "127.0.0.1";
  const std::uint16_t port =
      argc > 5 ? static_cast<std::uint16_t>(std::strtoul(argv[5], nullptr, 10))
               : 9000;

  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd < 0) {
    std::cerr << "socket: " << std::strerror(errno) << '\n';
    return 1;
  }

  sockaddr_in dest{};
  dest.sin_family = AF_INET;
  dest.sin_port = htons(port);
  if (inet_pton(AF_INET, host, &dest.sin_addr) != 1) {
    std::cerr << "bad address: " << host << '\n';
    return 1;
  }

  // connect() on a UDP socket fixes the default destination
  if (connect(fd, reinterpret_cast<sockaddr *>(&dest), sizeof(dest)) < 0) {
    std::cerr << "connect: " << std::strerror(errno) << '\n';
    return 1;
  }

  const std::size_t wire_size = exp::HDR_SIZE + payload;
  std::vector<std::byte> buf(wire_size);

  using clock = std::chrono::steady_clock;
  const auto t0 = clock::now();
  std::uint64_t sent = 0;
  std::uint64_t send_errors = 0;

  for (std::uint32_t i = 0; i < count; ++i) {
    if (rate > 0.0) {
      const auto target = t0 + std::chrono::nanoseconds(
                                   static_cast<std::int64_t>(i * (1e9 / rate)));
      std::this_thread::sleep_until(target);
    }
    exp::Packet pkt{.sequence_number = i, .send_ns = wire_now_ns()};
    exp::encode(buf, pkt);
    ssize_t n = send(fd, buf.data(), buf.size(), 0);
    if (n < 0) {
      ++send_errors;
    } else {
      ++sent;
    }
  }

  exp::Packet done{.sequence_number = 0xFFFFFFFFu, .send_ns = wire_now_ns()};
  exp::encode(buf, done);
  for (int k = 0; k < 5; ++k) {
    send(fd, buf.data(), buf.size(), 0);
  }

  const auto t1 = clock::now();
  const double secs = std::chrono::duration<double>(t1 - t0).count();
  std::cerr << "sent " << sent << " packets (" << send_errors << " errors) in "
            << secs << "s -> " << static_cast<std::uint64_t>(sent / secs)
            << " pps, " << (static_cast<double>(sent * wire_size) / secs / 1e6)
            << " MB/s wire\n";

  close(fd);
  return 0;
}
