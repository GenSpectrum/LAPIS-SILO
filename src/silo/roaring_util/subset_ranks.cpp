#include "silo/roaring_util/subset_ranks.h"

namespace silo::roaring_util {

// Get the subset of A & B and compute their ranks with respect to A
// Example:    A     B     rank in A
//             3
//             4 --- 4 --> 2
//             5 --- 5 --> 3
//                   6
//             7
//             9 --- 9 --> 5
//
// c_a and c_b should be non-empty
//
std::vector<uint64_t> roaringSubsetRanks(
   const roaring::internal::container_t* c_a,
   uint8_t type_a,
   const roaring::internal::container_t* c_b,
   uint8_t type_b,
   uint32_t base
) {
   uint8_t type_a_and_b;
   auto c_a_and_b = roaring::internal::container_and(c_a, type_a, c_b, type_b, &type_a_and_b);

   size_t cardinality = roaring::internal::container_get_cardinality(c_a_and_b, type_a_and_b);
   std::vector<uint32_t> a_and_b_as_vector(cardinality);

   roaring::internal::container_to_uint32_array(
      a_and_b_as_vector.data(), c_a_and_b, type_a_and_b, base
   );

   if (a_and_b_as_vector.empty()) {
      return {};
   }

   std::vector<uint64_t> ids_in_reconstructed_sequences(cardinality);
   roaring::internal::container_rank_many(
      c_a,
      type_a,
      base,
      a_and_b_as_vector.data(),
      a_and_b_as_vector.data() + cardinality,
      ids_in_reconstructed_sequences.data()
   );

   return ids_in_reconstructed_sequences;
}

}  // namespace silo::roaring_util
