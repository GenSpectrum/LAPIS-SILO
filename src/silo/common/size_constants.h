#pragma once

#include <cstddef>

namespace silo::common {

const size_t S_64_MB = 1 << 26;
const size_t S_16_MB = 1 << 24;
const size_t S_16_KB = 1 << 14;

static_assert(S_64_MB / 1024 / 1024 == 64);
static_assert(S_16_MB / 1024 / 1024 == 16);
static_assert(S_16_KB / 1024 == 16);

}  // namespace silo::common
