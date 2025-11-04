#pragma once

#include <optional>
#include <roaring/roaring.hh>

#include <roaring/roaring.h>

namespace silo::query_engine {

class BatchedBitmapReader {
  public:
   explicit BatchedBitmapReader(roaring::Roaring _bitmap, size_t _batch_size_minus_one)
       : bitmap(std::move(_bitmap)),
         cardinality(bitmap.cardinality()),
         batch_size_minus_one(_batch_size_minus_one) {
      ;
   }

   /**
    * @brief Attempts to get the next batch of row IDs.
    * @return An optional `roaring::Roaring` bitmap containing the batch of row IDs.
    * Returns `std::nullopt` if no more batches are available.
    */
   std::optional<roaring::Roaring> nextBatch();

  private:
   roaring::Roaring bitmap;
   size_t num_rows_produced = 0;
   size_t cardinality;  // Cache the cardinality for efficiency
   size_t batch_size_minus_one;
};

}  // namespace silo::query_engine
