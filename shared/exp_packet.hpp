#pragma once

#include "telemetry.hpp"
#include <cstdint>

struct packet {
  uint32_t sequence_number;
  uint64_t send_ns;
  uint32_t padding;
};
