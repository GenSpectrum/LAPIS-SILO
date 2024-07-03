#include "silo/common/numbers.h"

#include <stdexcept>

#include <fmt/format.h>

uint32_t uint64ToUint32(uint64_t val) {
   if (val <= UINT32_MAX) {
      return val;
   }
   throw std::overflow_error{fmt::format("number outside uint32 range: {}", val)};
}

uint32_t inc(uint32_t val) {
   if (val < UINT32_MAX) {
      return val + 1;
   }
   throw std::overflow_error{"inc: uint32 number overflow"};
}
