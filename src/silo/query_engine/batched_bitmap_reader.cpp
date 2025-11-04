#include "silo/query_engine/batched_bitmap_reader.h"
#include "silo/common/panic.h"

namespace silo::query_engine {

std::optional<roaring::Roaring> BatchedBitmapReader::nextBatch() {
   if (bitmap.isEmpty() || num_rows_produced >= cardinality) {
      return std::nullopt;
   }

   // Inclusive bounds [start_of_next_batch, end_of_next_batch]
   uint32_t start_of_next_batch;
   uint32_t end_of_next_batch;

   bool start_in_bitmap = bitmap.select(num_rows_produced, &start_of_next_batch);
   // Because `num_rows_produced < cardinality` an element with rank `num_rows_produced` must be in
   // the bitmap
   SILO_ASSERT(start_in_bitmap);

   size_t proposed_end_rank = num_rows_produced + batch_size_minus_one;
   bool end_selected = bitmap.select(proposed_end_rank, &end_of_next_batch);

   if (!end_selected) {
      // Fill batch with remainders. This is not empty because `num_rows_produced < cardinality`
      bitmap.select(cardinality - 1, &end_of_next_batch);
      num_rows_produced = cardinality;  // All remaining rows are processed
   } else {
      // A full batch can be formed.
      num_rows_produced += batch_size_minus_one + 1;
   }

   roaring::Roaring row_ids;
   // Make too large interval of [start_of_next_batch, end_of_next_batch] then intersect.
   // This is better than copying the original filter and then to the batch interval.
   row_ids.addRange(start_of_next_batch, end_of_next_batch + 1);
   row_ids &= bitmap;

   return row_ids;
}

}  // namespace silo::query_engine