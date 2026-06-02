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

void HorizontalCoverageIndex::insertCoverage(const Coverage& coverage) {
   const uint32_t sequence_idx = start_end.size();

   start_end.emplace_back(coverage.start, coverage.end);

   const uint16_t sequence_idx_upper_bits = sequence_idx >> 16;
   const uint16_t sequence_idx_lower_bits = sequence_idx & 0xFFFF;

   if (sequence_idx_lower_bits == 0) {
      batch_start_ends.emplace_back(coverage.start, coverage.end);
   } else {
      auto& [batch_start, batch_end] = batch_start_ends.back();
      batch_start = std::min(batch_start, coverage.start);
      batch_end = std::max(batch_end, coverage.end);
   }
   SILO_ASSERT_EQ(batch_start_ends.size(), static_cast<size_t>(sequence_idx_upper_bits) + 1);

   // We also have a row_wise bitmap, that covers all N symbols that are within the covered region
   roaring::Roaring horizontal_bitmap;
   horizontal_bitmap.addMany(coverage.missing_positions.size(), coverage.missing_positions.data());
   horizontal_bitmap.removeRange(0, coverage.start);
   horizontal_bitmap.removeRange(coverage.end, UINT32_MAX);
   horizontal_bitmap.runOptimize();
   horizontal_bitmap.shrinkToFit();

   if (horizontal_bitmap.cardinality() > 0) {
      horizontal_bitmaps.emplace(sequence_idx, std::move(horizontal_bitmap));
   }
}

void HorizontalCoverageIndex::insertNullSequence() {
   insertCoverage(Coverage{.start = 0, .end = 0, .missing_positions = {}});
}

template <typename SymbolType>
void HorizontalCoverageIndex::overwriteCoverageInSequence(
   std::vector<std::string>& sequences,
   const roaring::Roaring& row_ids
) const {
   uint32_t id_in_reconstructed_sequences = 0;
   for (const uint32_t row_id : row_ids) {
      const auto [start, end] = start_end.at(row_id);
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
