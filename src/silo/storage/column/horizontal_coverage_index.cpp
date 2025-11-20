#include "silo/storage/column/horizontal_coverage_index.h"

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/common/panic.h"

namespace silo::storage::column {

template <typename SymbolType>
void HorizontalCoverageIndex<SymbolType>::insertNullSequence() {
   start_end.emplace_back(0, 0);
}

template <typename SymbolType>
void HorizontalCoverageIndex<SymbolType>::insertCoverage(
   size_t sequence_idx,
   std::string sequence,
   uint32_t offset
) {
   std::optional<size_t> first_non_n_seen;
   std::optional<size_t> last_non_n_seen;

   std::vector<uint32_t> positions_with_symbol_missing;

   for (size_t char_idx_in_sequence = 0; char_idx_in_sequence < sequence.size();
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
      start_end.emplace_back(0, 0);
      positions_with_symbol_missing.clear();
      return;
   }

   size_t start_of_covered_region = first_non_n_seen.value();
   size_t end_of_covered_region_exclusive = last_non_n_seen.value() + 1;

   start_end.emplace_back(start_of_covered_region, end_of_covered_region_exclusive);

   if (not positions_with_symbol_missing.empty()) {
      horizontal_bitmaps[sequence_idx].addMany(
         positions_with_symbol_missing.size(), positions_with_symbol_missing.data()
      );
      horizontal_bitmaps[sequence_idx].removeRange(0, start_of_covered_region);
      horizontal_bitmaps[sequence_idx].removeRange(end_of_covered_region_exclusive, genome_length);
      horizontal_bitmaps[sequence_idx].runOptimize();
      horizontal_bitmaps[sequence_idx].shrinkToFit();
   }
}

template <typename SymbolType>
void HorizontalCoverageIndex<SymbolType>::overwriteCoverageInSequence(
   std::vector<std::string>& sequences,
   const roaring::Roaring& row_ids
) const {
   size_t id_in_reconstructed_sequences = 0;
   for (size_t row_id : row_ids) {
      const auto [start, end] = start_end.at(row_id);
      for (size_t position_idx = 0; position_idx < start; position_idx++) {
         sequences.at(id_in_reconstructed_sequences).at(position_idx) =
            SymbolType::symbolToChar(SymbolType::SYMBOL_MISSING);
      }
      for (size_t position_idx = end; position_idx < genome_length; position_idx++) {
         sequences.at(id_in_reconstructed_sequences).at(position_idx) =
            SymbolType::symbolToChar(SymbolType::SYMBOL_MISSING);
      }

      auto iter = horizontal_bitmaps.find(row_id);
      if (iter != horizontal_bitmaps.end()) {
         const roaring::Roaring& n_bitmap = iter->second;
         for (const size_t position_idx : n_bitmap) {
            sequences.at(id_in_reconstructed_sequences).at(position_idx) =
               SymbolType::symbolToChar(SymbolType::SYMBOL_MISSING);
         }
      }
      id_in_reconstructed_sequences++;
   }
}

template class HorizontalCoverageIndex<Nucleotide>;
template class HorizontalCoverageIndex<AminoAcid>;

}  // namespace silo::storage::column
