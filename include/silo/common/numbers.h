#pragma once

#include <cstdint>

/// Throws `std::overflow_error` if `val` isn't in uint32_t range.
uint32_t uint64ToUint32(uint64_t val);

/// Throws `std::overflow_error` on overflow
uint32_t inc(uint32_t val);
