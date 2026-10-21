#pragma once

#include <cstddef>
#include <functional>
#include <vector>

#include <nlohmann/json.hpp>

#include "rhydb/storage/column/column.h"

namespace rhydb::test {

/// Mirror of the ingestion chunk size. Datasets larger than this span multiple
/// chunks, which forces the cross-chunk aggregation paths to run.
constexpr size_t CHUNK_SIZE = storage::column::COLUMN_CHUNK_SIZE;

/// A generated dataset whose rows are laid out to span multiple ingestion
/// chunks, plus bookkeeping needed to assert the expected aggregation result.
struct LargeDataset {
   /// Real rows interleaved with filler rows, flattened in order.
   std::vector<nlohmann::json> rows;
   /// Total number of filler rows added for padding.
   size_t filler_count = 0;
};

/// Flattens `chunks` into one dataset that spans exactly `chunks.size()`
/// ingestion chunks.
///
/// Each inner vector holds the real rows for one chunk, in order. Every chunk
/// except the last is padded up to CHUNK_SIZE with filler rows, which forces
/// that chunk to flush at the chunk boundary. The last chunk is left partial,
/// so the non-full final chunk path is exercised too.
///
/// `make_filler(chunk_index, filler_index)` must return a schema-valid row for
/// the test's schema. `chunk_index` is the chunk the filler row lands in, and
/// `filler_index` is the running filler count across all chunks (use it to
/// generate a unique primary key). Filler rows should group predictably (e.g. a
/// null sequence collapses them into the single null group), so the test can
/// assert their combined count.
inline LargeDataset buildAcrossChunks(
   const std::vector<std::vector<nlohmann::json>>& chunks,
   const std::function<nlohmann::json(size_t chunk_index, size_t filler_index)>& make_filler
) {
   LargeDataset dataset;
   for (size_t chunk_index = 0; chunk_index < chunks.size(); ++chunk_index) {
      const auto& chunk_rows = chunks[chunk_index];
      for (const auto& row : chunk_rows) {
         dataset.rows.push_back(row);
      }

      const bool is_last_chunk = chunk_index + 1 == chunks.size();
      if (is_last_chunk) {
         continue;
      }

      for (size_t i = chunk_rows.size(); i < CHUNK_SIZE; ++i) {
         dataset.rows.push_back(make_filler(chunk_index, dataset.filler_count));
         ++dataset.filler_count;
      }
   }
   return dataset;
}

}  // namespace rhydb::test
