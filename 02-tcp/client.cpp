#include "telemetry.hpp"

#include <arpa/inet.h>
#include <array>
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

/**
@brief writes all bytes in teh buffer to the fd
 */
static bool send_all(int fd, std::span<const std::byte> buffer) {
  size_t total = 0;
  while (total < buffer.size()) {
    ssize_t n = send(fd, buffer.data() + total, buffer.size() - total, 0);
    if (n <= 0) {
      return false;
    }
    total += static_cast<size_t>(n);
  }
  return true;
}

int main(int argc, char **argv) {
  const int count = argc > 1 ? std::atoi(argv[1]) : 10;
  // Creates a socket, which returns a file descriptor an index into the
  // kernel's per process fd table Fd lives in kernel
  int fd = socket(AF_INET, SOCK_STREAM, 0); // AF_NET is IPv4, SOCK_DGRAM is UDP
  if (fd < 0) {
    std::cerr << "socket: " << std::strerror(errno) << '\n';
    return 1;
  }

  // Destintaiton of socket
  sockaddr_in dest{};
  dest.sin_family = AF_INET;
  dest.sin_port = htons(9100); // Big endian

  if (inet_pton(AF_INET, "127.0.0.1", &dest.sin_addr) != 1) {
    std::cerr << "bad address\n";
    return 1;
  }
  // Connect to addr
  if (connect(fd, reinterpret_cast<sockaddr *>(&dest), sizeof(dest)) < 0) {
    std::cerr << "connect: " << std::strerror(errno) << '\n';
    return 1;
  }

  for (int i{}; i < count; i++) {
    std::array<std::byte, MESSAGE_SIZE> payload{};
    telemetry::write<std::uint32_t>(payload, 0, 42);
    telemetry::write<std::uint32_t>(payload, 4, 7);
    telemetry::write<std::int32_t>(payload, 8, 87);

    std::array<std::byte, 4 + MESSAGE_SIZE> frame{};
    telemetry::write<std::uint32_t>(frame, 0, MESSAGE_SIZE);
    std::memcpy(frame.data() + 4, payload.data(), MESSAGE_SIZE);

    if (!send_all(fd, frame)) {
      std::cerr << "send_all failed at frame " << i << '\n';
      break;
    }
  }

  close(fd);
  return 0;
}
