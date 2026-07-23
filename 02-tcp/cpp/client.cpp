#include "telemetry.hpp"

#include <arpa/inet.h>
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

int main() {
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
  dest.sin_port = htons(9000); // Big endian

  if (inet_pton(AF_INET, "127.0.0.1", &dest.sin_addr) != 1) {
    std::cerr << "bad address\n";
    return 1;
  }

  std::array<std::byte, MESSAGE_SIZE> buffer{};
  telemetry::write<std::uint32_t>(buffer, 0, 42);
  telemetry::write<std::uint32_t>(buffer, 4, 7);
  telemetry::write<std::int32_t>(buffer, 8, 87);
  print_buffer(buffer);

  connect(fd, reinterpret_cast<sockaddr *>(&dest.sin_addr), sizeof(dest));

  close(fd);
  return 0;
}
