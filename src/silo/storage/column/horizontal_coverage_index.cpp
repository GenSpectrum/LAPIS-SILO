#include "silo/storage/column/horizontal_coverage_index.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>

#include <spdlog/spdlog.h>
#include <roaring/roaring.hh>

#include "silo/common/aa_symbols.h"
#include "silo/common/aligned_sequence.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/common/panic.h"

namespace silo::storage::column {

void HorizontalCoverageIndex::insertCoverage(RowId row_id, const Coverage& coverage) {
   if (row_id.chunk_id == batch_start_ends.size()) {
      batch_start_ends.emplace_back(UINT32_MAX, 0);
   }
   if (row_id.chunk_id == start_end.size()) {
      start_end.emplace_back();
   }
   // For now, coverage needs to be inserted in ascending order
   SILO_ASSERT_EQ(batch_start_ends.size(), start_end.size());
   SILO_ASSERT(row_id.chunk_id == start_end.size() - 1);
   SILO_ASSERT_EQ(row_id.row_in_chunk, start_end.at(row_id.chunk_id).size());

   start_end.at(row_id.chunk_id).emplace_back(coverage.start, coverage.end);

   auto& [batch_start, batch_end] = batch_start_ends.back();
   batch_start = std::min(batch_start, coverage.start);
   batch_end = std::max(batch_end, coverage.end);

   // We also have a row_wise bitmap, that covers all N symbols that are within the covered region
   roaring::Roaring horizontal_bitmap;
   horizontal_bitmap.addMany(coverage.missing_positions.size(), coverage.missing_positions.data());
   horizontal_bitmap.removeRange(0, coverage.start);
   horizontal_bitmap.removeRange(coverage.end, UINT32_MAX);
   horizontal_bitmap.runOptimize();
   horizontal_bitmap.shrinkToFit();

   if (horizontal_bitmap.cardinality() > 0) {
      horizontal_bitmaps.emplace(row_id.toGlobal(), std::move(horizontal_bitmap));
   }
}

void HorizontalCoverageIndex::insertNullSequence(RowId row_id) {
   insertCoverage(row_id, Coverage{.start = 0, .end = 0, .missing_positions = {}});
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
std::vector<uint64_t> HorizontalCoverageIndex::computeCoverageCardinalities(size_t genome_length
) const {
   std::vector<int64_t> coverage_changes(genome_length + 1, 0);
   for (const auto& chunk : start_end) {
      for (const auto& [start, end] : chunk) {
         SILO_ASSERT_LE(end, genome_length);
         coverage_changes[start] += 1;
         coverage_changes[end] -= 1;
      }
   }
   for (const auto& [_row_id, missing_positions] : horizontal_bitmaps) {
      for (const uint32_t position_idx : missing_positions) {
         SILO_ASSERT_LT(position_idx, genome_length);
         coverage_changes[position_idx] -= 1;
         coverage_changes[position_idx + 1] += 1;
      }
   }

   std::vector<uint64_t> cardinalities(genome_length);
   uint64_t cardinality = 0;
   for (size_t position_idx = 0; position_idx < genome_length; ++position_idx) {
      cardinality += coverage_changes[position_idx];
      SILO_ASSERT_GE(cardinality, 0UL);
      cardinalities[position_idx] = static_cast<uint32_t>(cardinality);
   }
   return cardinalities;
}

template <typename SymbolType>
void HorizontalCoverageIndex::overwriteCoverageInSequence(
   std::vector<std::string>& sequences,
   const roaring::Roaring& row_ids
) const {
   uint32_t id_in_reconstructed_sequences = 0;
   for (const uint32_t row_id : row_ids) {
      const auto [start, end] = coverageRange(row_id);
      const size_t sequence_size = sequences.at(id_in_reconstructed_sequences).size();

      for (uint32_t position_idx = 0; position_idx < start; position_idx++) {
         sequences.at(id_in_reconstructed_sequences).at(position_idx) =
            SymbolType::symbolToChar(SymbolType::SYMBOL_MISSING);
      }
      for (uint32_t position_idx = end; position_idx < sequence_size; position_idx++) {
         sequences.at(id_in_reconstructed_sequences).at(position_idx) =
            SymbolType::symbolToChar(SymbolType::SYMBOL_MISSING);
      }

      auto iter = horizontal_bitmaps.find(row_id);
      if (iter != horizontal_bitmaps.end()) {
         const roaring::Roaring& n_bitmap = iter->second;
         for (const uint32_t position_idx : n_bitmap) {
            sequences.at(id_in_reconstructed_sequences).at(position_idx) =
               SymbolType::symbolToChar(SymbolType::SYMBOL_MISSING);
         }
      }
      id_in_reconstructed_sequences++;
   }
}

template void HorizontalCoverageIndex::overwriteCoverageInSequence<Nucleotide>(
   std::vector<std::string>& sequences,
   const roaring::Roaring& row_ids
) const;
template void HorizontalCoverageIndex::overwriteCoverageInSequence<AminoAcid>(
   std::vector<std::string>& sequences,
   const roaring::Roaring& row_ids
) const;

}  // namespace silo::storage::column
