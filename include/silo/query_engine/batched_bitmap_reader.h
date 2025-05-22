#pragma once

#include <iostream>
#include <memory>
#include <optional>

#include <roaring/roaring.h>

#include "silo/query_engine/copy_on_write_bitmap.h"

namespace silo::query_engine {

class BatchedBitmapReader {
  public:
   explicit BatchedBitmapReader(CopyOnWriteBitmap filter, size_t batch_size_minus_one)
       : filter(std::move(filter)),
         num_rows_produced(0),
         cardinality(0),
         batch_size_minus_one(batch_size_minus_one) {
      cardinality = this->filter->cardinality();
   }

   /**
    * @brief Attempts to get the next batch of row IDs.
    * @return An optional `roaring::Roaring` bitmap containing the batch of row IDs.
    * Returns `std::nullopt` if no more batches are available.
    */
   std::optional<roaring::Roaring> nextBatch();

  private:
   CopyOnWriteBitmap filter;
   size_t num_rows_produced;
   size_t cardinality;  // Cache the cardinality for efficiency
   size_t batch_size_minus_one;
};

}  // namespace silo::query_engine
