#pragma once

#include <vector>

#include <roaring/roaring.hh>

namespace silo::roaring_util {

std::vector<uint64_t> roaringSubsetRanks(
   const roaring::internal::container_t* c_a,
   uint8_t type_a,
   const roaring::internal::container_t* c_b,
   uint8_t type_b,
   uint32_t base
);

}  // namespace silo::roaring_util
