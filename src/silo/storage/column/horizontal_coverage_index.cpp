#include "silo/storage/column/horizontal_coverage_index.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>

#include <spdlog/spdlog.h>
#include <roaring/roaring.hh>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/common/panic.h"

namespace silo::storage::column {

void HorizontalCoverageIndex::insertCoverage(
   uint32_t start,
   uint32_t end,
   const std::vector<uint32_t>& positions_with_symbol_missing
) {
   uint32_t sequence_idx = start_end.size();

   start_end.emplace_back(start, end);

   uint16_t sequence_idx_upper_bits = sequence_idx >> 16;
   uint16_t sequence_idx_lower_bits = sequence_idx & 0xFFFF;

   if (sequence_idx_lower_bits == 0) {
      batch_start_ends.emplace_back(start, end);
   } else {
      auto& [batch_start, batch_end] = batch_start_ends.back();
      batch_start = std::min(batch_start, start);
      batch_end = std::max(batch_end, end);
   }
   SILO_ASSERT_EQ(batch_start_ends.size(), sequence_idx_upper_bits + 1);

   // We also have a row_wise bitmap, that covers all N symbols that are within the covered region
   roaring::Roaring horizontal_bitmap;
   horizontal_bitmap.addMany(
      positions_with_symbol_missing.size(), positions_with_symbol_missing.data()
   );
   horizontal_bitmap.removeRange(0, start);
   horizontal_bitmap.removeRange(end, UINT32_MAX);
   horizontal_bitmap.runOptimize();
   horizontal_bitmap.shrinkToFit();

   if (horizontal_bitmap.cardinality() > 0) {
      horizontal_bitmaps.emplace(sequence_idx, std::move(horizontal_bitmap));
   }
}

void HorizontalCoverageIndex::insertNullSequence() {
   insertCoverage(0, 0, {});
}

template <typename SymbolType>
void HorizontalCoverageIndex::insertSequenceCoverage(std::string sequence, uint32_t offset) {
   std::optional<uint32_t> first_non_n_seen;
   std::optional<uint32_t> last_non_n_seen;

   std::vector<uint32_t> positions_with_symbol_missing;

   SILO_ASSERT(sequence.size() + offset < UINT32_MAX);

   for (uint32_t char_idx_in_sequence = 0; char_idx_in_sequence < sequence.size();
        ++char_idx_in_sequence) {
      const char character = sequence[char_idx_in_sequence];
      size_t position_idx = char_idx_in_sequence + offset;
      const auto symbol = SymbolType::charToSymbol(character);
      if (symbol == SymbolType::SYMBOL_MISSING) {
         positions_with_symbol_missing.push_back(position_idx);
      } else {
         if (not first_non_n_seen.has_value()) {
            first_non_n_seen = position_idx;
         }
         last_non_n_seen = position_idx;
      }
   }

   // Either both are std::nullopt or neither is
   SILO_ASSERT_EQ(first_non_n_seen.has_value(), last_non_n_seen.has_value());

   if (not first_non_n_seen.has_value()) {
      insertCoverage(0, 0, {});
      return;
   }

   uint32_t start_of_covered_region = first_non_n_seen.value();
   uint32_t end_of_covered_region_exclusive = last_non_n_seen.value() + 1;

   insertCoverage(
      start_of_covered_region, end_of_covered_region_exclusive, positions_with_symbol_missing
   );
}
template void HorizontalCoverageIndex::insertSequenceCoverage<Nucleotide>(
   std::string sequence,
   uint32_t offset
);
template void HorizontalCoverageIndex::insertSequenceCoverage<AminoAcid>(
   std::string sequence,
   uint32_t offset
);

template <typename SymbolType>
void HorizontalCoverageIndex::overwriteCoverageInSequence(
   std::vector<std::string>& sequences,
   const roaring::Roaring& row_ids
) const {
   uint32_t id_in_reconstructed_sequences = 0;
   for (uint32_t row_id : row_ids) {
      const auto [start, end] = start_end.at(row_id);
      size_t sequence_size = sequences.at(id_in_reconstructed_sequences).size();

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
