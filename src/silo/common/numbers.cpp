#include "silo/common/numbers.h"

#include <stdexcept>

namespace silo::common {

uint32_t add1(uint32_t val) {
   if (val < UINT32_MAX) {
      return val + 1;
   }
   throw std::overflow_error{"add1: uint32 number overflow"};
}

}  // namespace silo::common
