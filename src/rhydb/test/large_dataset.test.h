#pragma once

#include <cstddef>
#include <functional>
#include <utility>
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
   /// All rows, flattened in order.
   std::vector<nlohmann::json> rows;
   /// Number of template-only rows (padding rows with no override from
   /// `values`). With a template that groups predictably (e.g. a null
   /// sequence), these collapse into a single group whose count is asserted.
   size_t filler_count = 0;
};

/// Flattens `values` into one dataset that spans exactly `values.size()`
/// ingestion chunks.
///
/// Every row starts from `make_template(chunk_index, row_index)`, which must
/// return a complete, schema-valid row (including a unique primary key). For
/// each chunk, the first `values[chunk_index].size()` rows additionally have
/// the columns in the corresponding `values[chunk_index][row_index]` overlaid
/// on top of the template (top-level keys are overwritten).
///
/// Every chunk except the last is padded up to CHUNK_SIZE with template-only
/// rows, which forces that chunk to flush at the chunk boundary. The last chunk
/// holds exactly `values.back().size()` rows and is left partial, so the
/// non-full final chunk path is exercised too.
inline LargeDataset buildLargeTestDataset(
   const std::function<nlohmann::json(size_t chunk_index, size_t row_index)>& make_template,
   const std::vector<std::vector<nlohmann::json>>& values
) {
   LargeDataset dataset;
   for (size_t chunk_index = 0; chunk_index < values.size(); ++chunk_index) {
      const auto& chunk_values = values[chunk_index];
      const bool is_last_chunk = chunk_index + 1 == values.size();
      const size_t rows_in_chunk = is_last_chunk ? chunk_values.size() : CHUNK_SIZE;

      for (size_t row_index = 0; row_index < rows_in_chunk; ++row_index) {
         nlohmann::json row = make_template(chunk_index, row_index);
         if (row_index < chunk_values.size()) {
            row.update(chunk_values[row_index]);
         } else {
            ++dataset.filler_count;
         }
         dataset.rows.push_back(std::move(row));
      }
   }
   return dataset;
}

}  // namespace rhydb::test
