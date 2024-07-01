#include "silo/common/numbers.h"

#include <stdexcept>

uint32_t inc(uint32_t val) {
   if (val < UINT32_MAX) {
      return val + 1;
   }
   throw std::overflow_error{"inc: uint32 number overflow"};
}
