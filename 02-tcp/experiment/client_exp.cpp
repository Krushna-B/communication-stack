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
#include <span>
#include <vector>

static std::uint64_t wire_now_ns() {
  timespec ts{};
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return static_cast<std::uint64_t>(ts.tv_sec) * 1'000'000'000ULL +
         static_cast<std::uint64_t>(ts.tv_nsec);
}

static bool send_all(int fd, std::span<const std::byte> buf) {
  std::size_t total = 0;
  while (total < buf.size()) {
    ssize_t n = send(fd, buf.data() + total, buf.size() - total, 0);
    if (n <= 0) {
      return false;
    }
    total += static_cast<std::size_t>(n);
  }
  return true;
}

int main(int argc, char **argv) {
  const std::uint32_t count =
      argc > 1 ? static_cast<std::uint32_t>(std::strtoul(argv[1], nullptr, 10))
               : 100000;
  const std::size_t payload =
      argc > 2 ? static_cast<std::size_t>(std::strtoul(argv[2], nullptr, 10))
               : 20;
  const char *host = argc > 3 ? argv[3] : "127.0.0.1";
  const std::uint16_t port =
      argc > 4 ? static_cast<std::uint16_t>(std::strtoul(argv[4], nullptr, 10))
               : 9100;

  int fd = socket(AF_INET, SOCK_STREAM, 0);
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
  if (connect(fd, reinterpret_cast<sockaddr *>(&dest), sizeof(dest)) < 0) {
    std::cerr << "connect: " << std::strerror(errno) << '\n';
    return 1;
  }

  const std::size_t wire = exp::HDR_SIZE + payload;
  std::vector<std::byte> frame(4 + wire);
  telemetry::write<std::uint32_t>(frame, 0, static_cast<std::uint32_t>(wire));

  using clock = std::chrono::steady_clock;
  const auto t0 = clock::now();
  std::uint64_t block_ns = 0;

  for (std::uint32_t i = 0; i < count; ++i) {
    exp::Packet pkt{.sequence_number = i, .send_ns = wire_now_ns()};
    exp::encode(std::span<std::byte>(frame).subspan(4), pkt);
    const auto b0 = clock::now();
    if (!send_all(fd, frame)) {
      std::cerr << "send_all failed at " << i << '\n';
      break;
    }
    block_ns += static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(clock::now() - b0)
            .count());
  }

  const auto t1 = clock::now();
  const double secs = std::chrono::duration<double>(t1 - t0).count();
  std::cerr << "sent " << count << " frames in " << secs << "s -> "
            << static_cast<std::uint64_t>(count / secs) << " msg/s, "
            << (static_cast<double>(count * frame.size()) / secs / 1e6)
            << " MB/s, send-blocked " << (static_cast<double>(block_ns) / 1e6)
            << " ms total (" << (static_cast<double>(block_ns) / count)
            << " ns/msg)\n";

  close(fd);
  return 0;
}
