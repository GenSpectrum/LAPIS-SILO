#include "rhydb/storage/column/horizontal_coverage_index.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>

#include <spdlog/spdlog.h>
#include <roaring/roaring.hh>

#include "rhydb/common/aa_symbols.h"
#include "rhydb/common/aligned_sequence.h"
#include "rhydb/common/nucleotide_symbols.h"
#include "rhydb/common/panic.h"

namespace rhydb::storage::column {

void HorizontalCoverageIndex::insertCoverage(RowId row_id, const Coverage& coverage) {
   if (row_id.chunk_id == starts.size()) {
      starts.emplace_back();
      ends.emplace_back();
      batch_min_start.emplace_back(UINT32_MAX);
      batch_max_start.emplace_back(0);
      batch_min_end.emplace_back(UINT32_MAX);
      batch_max_end.emplace_back(0);
   }
   // For now, coverage needs to be inserted in ascending order
   SILO_ASSERT_EQ(starts.size(), ends.size());
   SILO_ASSERT(row_id.chunk_id == starts.size() - 1);
   SILO_ASSERT_EQ(row_id.row_in_chunk, starts.at(row_id.chunk_id).size());

   starts.at(row_id.chunk_id).push_back(coverage.start);
   ends.at(row_id.chunk_id).push_back(coverage.end);

   batch_min_start.back() = std::min(batch_min_start.back(), coverage.start);
   batch_max_start.back() = std::max(batch_max_start.back(), coverage.start);
   batch_min_end.back() = std::min(batch_min_end.back(), coverage.end);
   batch_max_end.back() = std::max(batch_max_end.back(), coverage.end);

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

roaring_util::RoaringContainer HorizontalCoverageIndex::coveredRowsInChunk(
   uint32_t position,
   uint16_t chunk_id
) const {
   roaring::Roaring result;
   if (chunk_id >= starts.size()) {
      return roaring_util::RoaringContainer{};
   }

   // No row in this chunk can cover the position.
   if (batch_max_end.at(chunk_id) <= position || batch_min_start.at(chunk_id) > position) {
      return roaring_util::RoaringContainer{};
   }

   const uint32_t base_row_id = static_cast<uint32_t>(chunk_id) << 16U;
   const auto& chunk_starts = starts[chunk_id];
   const auto& chunk_ends = ends[chunk_id];

   // Fast path: if the position lies within the chunk's intersection envelope
   // `[batch_max_start, batch_min_end)`, every row in the chunk covers it, so add the whole chunk in
   // one range operation.
   if (batch_max_start.at(chunk_id) <= position && position < batch_min_end.at(chunk_id)) {
      result.addRange(base_row_id, base_row_id + chunk_starts.size());
   } else {
      roaring_util::BitmapBuilderByRange partial_builder;
      for (size_t row_in_chunk = 0; row_in_chunk < chunk_starts.size(); ++row_in_chunk) {
         if (chunk_starts[row_in_chunk] <= position && position < chunk_ends[row_in_chunk]) {
            partial_builder.add(base_row_id | static_cast<uint32_t>(row_in_chunk));
         }
      }
      result |= std::move(partial_builder).getBitmap();
   }

   // Remove this chunk's in-region N positions: a row whose covered range includes `position` but
   // records an N there is not covered at `position` (it belongs to the missing symbol instead).
   const auto chunk_rows_begin = horizontal_bitmaps.lower_bound(base_row_id);
   const uint64_t chunk_end_key = static_cast<uint64_t>(base_row_id) + chunk_starts.size();
   const auto chunk_rows_end =
      chunk_end_key > UINT32_MAX
         ? horizontal_bitmaps.end()
         : horizontal_bitmaps.lower_bound(static_cast<uint32_t>(chunk_end_key));
   for (auto iter = chunk_rows_begin; iter != chunk_rows_end; ++iter) {
      if (iter->second.contains(position)) {
         result.remove(iter->first);
      }
   }

   // Every row added above lies in `chunk_id`, so `result` holds at most this one 2^16 container.
   // Steal it out of the roaring array (no clone) and hand it back on its own; an empty result
   // becomes an empty container.
   auto& roaring_array = result.roaring.high_low_container;
   if (roaring_array.size == 0) {
      return roaring_util::RoaringContainer{};
   }
   SILO_ASSERT_EQ(roaring_array.size, 1);
   const auto cardinality = static_cast<uint32_t>(roaring::internal::container_get_cardinality(
      roaring_array.containers[0], roaring_array.typecodes[0]
   ));
   roaring_util::RoaringContainer container{
      roaring_array.containers[0], cardinality, roaring_array.typecodes[0]
   };
   roaring::internal::ra_clear_without_containers(&roaring_array);
   return container;
}

bool HorizontalCoverageIndex::noRowCoversPositionInChunk(uint32_t position, uint16_t chunk_id)
   const {
   if (chunk_id >= start_end.size()) {
      return true;
   }
   const auto [batch_start, batch_end] = batch_start_ends.at(chunk_id);
   return batch_end <= position || batch_start > position;
}

bool HorizontalCoverageIndex::positionCoveredByWholeChunk(uint32_t position, uint16_t chunk_id)
   const {
   if (chunk_id >= start_end.size() || batch_covered_intersection.size() != start_end.size()) {
      return false;
   }
   const auto [covered_max_start, covered_min_end] = batch_covered_intersection[chunk_id];
   // The envelope says every row's covered range includes `position` iff it lies in the
   // intersection of all those ranges. (A null row has range [0, 0), forcing `covered_min_end` to
   // 0, so this is always false for a chunk with nulls.)
   if (position < covered_max_start || position >= covered_min_end) {
      return false;
   }
   // Each row covers `position` only as the reference symbol if it records no in-region N there, so
   // reject the chunk if any of its in-region-N rows carries an N at `position`.
   const uint32_t base_row_id = static_cast<uint32_t>(chunk_id) << 16U;
   const uint64_t chunk_end_key = static_cast<uint64_t>(base_row_id) + start_end[chunk_id].size();
   const auto chunk_rows_begin = horizontal_bitmaps.lower_bound(base_row_id);
   const auto chunk_rows_end =
      chunk_end_key > UINT32_MAX
         ? horizontal_bitmaps.end()
         : horizontal_bitmaps.lower_bound(static_cast<uint32_t>(chunk_end_key));
   for (auto iter = chunk_rows_begin; iter != chunk_rows_end; ++iter) {
      if (iter->second.contains(position)) {
         return false;
      }
   }
   return true;
}

void HorizontalCoverageIndex::insertNullSequence(RowId row_id) {
   insertCoverage(row_id, Coverage{.start = 0, .end = 0, .missing_positions = {}});
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
std::vector<uint64_t> HorizontalCoverageIndex::computeCoverageCardinalities(size_t genome_length
) const {
   std::vector<int64_t> coverage_changes(genome_length + 1, 0);
   for (size_t chunk_id = 0; chunk_id < starts.size(); ++chunk_id) {
      const auto& chunk_starts = starts[chunk_id];
      const auto& chunk_ends = ends[chunk_id];
      for (size_t row_in_chunk = 0; row_in_chunk < chunk_starts.size(); ++row_in_chunk) {
         const uint32_t start = chunk_starts[row_in_chunk];
         const uint32_t end = chunk_ends[row_in_chunk];
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

}  // namespace rhydb::storage::column
