#pragma once

#include <cstdint>

namespace rhydb::common {

/// Throws `std::overflow_error` on overflow
uint32_t add1(uint32_t val);

}  // namespace rhydb::common
