#include "silo/common/numbers.h"

#include <cassert>

uint32_t uint64ToUint32(uint64_t val) {
   assert(val <= UINT32_MAX);
   return val;
}

uint32_t inc(uint32_t val) {
   assert(val < UINT32_MAX);
   return val + 1;
}
