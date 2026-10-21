#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "rhydb/storage/column/column.h"
#include "rhydb/storage/reference_genomes.h"

namespace rhydb::test {

/// Mirror of the ingestion chunk size. Datasets larger than this span multiple
/// chunks, which forces the cross-chunk aggregation paths to run.
constexpr size_t CHUNK_SIZE = storage::column::COLUMN_CHUNK_SIZE;

/// A generated dataset whose rows are laid out to span multiple ingestion
/// chunks, plus bookkeeping needed to assert the expected aggregation result.
struct LargeDataset {
   /// Real rows interleaved with null-sequence filler rows, flattened in order.
   std::vector<nlohmann::json> rows;
   /// Total number of null-sequence filler rows added for padding. All filler
   /// rows have a null `segment1`, so they collapse into the single `s1 = null`
   /// group when grouping by `segment1.at(...)`.
   size_t filler_count = 0;
};

/// Shared database config for large-dataset tests: a string primary key and a
/// single nucleotide segment `segment1`.
inline const std::string LARGE_DATASET_CONFIG =
   R"(
schema:
  instanceName: "large dataset"
  metadata:
    - name: "primaryKey"
      type: "string"
  primaryKey: "primaryKey"
)";

/// Reference genomes matching LARGE_DATASET_CONFIG. Reference is intentionally
/// short to keep large datasets cheap in memory and time.
inline const ReferenceGenomes& largeDatasetReferenceGenomes() {
   static const ReferenceGenomes reference_genomes{
      {{"segment1", "ATGCN"}},
      {},
   };
   return reference_genomes;
}

/// Builds a real row carrying `nucleotide_sequence` on `segment1`. The caller
/// controls which group the row falls into via the symbols in the sequence
/// (e.g. the symbol at position 1 for `segment1.at(1)`).
inline nlohmann::json largeDatasetRow(
   const std::string& primary_key,
   const std::string& nucleotide_sequence
) {
   return {
      {"primaryKey", primary_key},
      {"segment1", {{"sequence", nucleotide_sequence}, {"insertions", nlohmann::json::array()}}},
   };
}

/// Builds a filler row with a null `segment1`. Filler rows group as `s1 = null`.
inline nlohmann::json largeDatasetFillerRow(const std::string& primary_key) {
   return {
      {"primaryKey", primary_key},
      {"segment1", nullptr},
   };
}

/// Flattens `chunks` into one dataset that spans exactly `chunks.size()`
/// ingestion chunks.
///
/// Each inner vector holds the real rows for one chunk, in order. Every chunk
/// except the last is padded up to CHUNK_SIZE with null-sequence filler rows,
/// which forces that chunk to flush at the chunk boundary. The last chunk is
/// left partial, so the non-full final chunk path is exercised too.
///
/// Each real row must already carry a unique `primaryKey`. Filler rows get
/// generated keys of the form `filler_<n>`.
inline LargeDataset buildAcrossChunks(const std::vector<std::vector<nlohmann::json>>& chunks) {
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
         dataset.rows.push_back(
            largeDatasetFillerRow("filler_" + std::to_string(dataset.filler_count))
         );
         ++dataset.filler_count;
      }
   }
   return dataset;
}

}  // namespace rhydb::test
