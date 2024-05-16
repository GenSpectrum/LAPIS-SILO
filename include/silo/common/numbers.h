#pragma once

#include <cstdint>

/// Abort if `val` isn't in uint32_t range.
uint32_t uint64ToUint32(uint64_t val);

/// Abort on overflow
uint32_t inc(uint32_t val);
