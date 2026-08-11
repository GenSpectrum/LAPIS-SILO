#include "rhydb/storage/column/horizontal_coverage_index.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>

#include <spdlog/spdlog.h>
#include <roaring/roaring.hh>

// AVX-512 is x86-only; the project also targets Apple-Silicon macOS and wasm, where <immintrin.h>,
// the `target` attribute and `__builtin_cpu_supports` do not exist. Everything AVX-512 lives behind
// this guard, with a scalar fallback used everywhere else.
#if defined(__x86_64__) || defined(_M_X64)
#include <immintrin.h>
#define SILO_HAS_X86_SIMD 1
#endif

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

namespace {

/// Sets bit `row` of `bitset` for every row where `starts[row] <= position < ends[row]`, returning
/// the number of such rows. `bitset` must be a zeroed 2^16-bit (1024-word) buffer.
uint32_t coverageScanScalar(
   const uint32_t* starts,
   const uint32_t* ends,
   size_t num_rows,
   uint32_t position,
   uint64_t* bitset
) {
   uint32_t cardinality = 0;
   for (size_t row = 0; row < num_rows; ++row) {
      if (starts[row] <= position && position < ends[row]) {
         bitset[row >> 6U] |= (uint64_t{1} << (row & 63U));
         ++cardinality;
      }
   }
   return cardinality;
}

#ifdef SILO_HAS_X86_SIMD
/// AVX-512 version: 16 rows per iteration. Each 16-bit compare mask lands whole inside one 64-bit
/// bitset word (row indices step by 16, so the bit offset is one of 0/16/32/48). Compiled behind a
/// per-function `target` attribute + runtime `__builtin_cpu_supports` dispatch so the binary stays
/// runnable on x86 without AVX-512.
__attribute__((target("avx512f,avx512bw"))) uint32_t coverageScanAvx512(
   const uint32_t* starts,
   const uint32_t* ends,
   size_t num_rows,
   uint32_t position,
   uint64_t* bitset
) {
   const __m512i position_vec = _mm512_set1_epi32(static_cast<int>(position));
   uint32_t cardinality = 0;
   size_t row = 0;
   for (; row + 16 <= num_rows; row += 16) {
      const __m512i starts_vec = _mm512_loadu_si512(static_cast<const void*>(starts + row));
      const __m512i ends_vec = _mm512_loadu_si512(static_cast<const void*>(ends + row));
      const __mmask16 start_le = _mm512_cmp_epu32_mask(starts_vec, position_vec, _MM_CMPINT_LE);
      const __mmask16 end_gt = _mm512_cmp_epu32_mask(ends_vec, position_vec, _MM_CMPINT_NLE);
      const __mmask16 covered = start_le & end_gt;
      bitset[row >> 6U] |= (static_cast<uint64_t>(covered) << (row & 63U));
      cardinality += static_cast<uint32_t>(__builtin_popcount(covered));
   }
   for (; row < num_rows; ++row) {
      if (starts[row] <= position && position < ends[row]) {
         bitset[row >> 6U] |= (uint64_t{1} << (row & 63U));
         ++cardinality;
      }
   }
   return cardinality;
}

bool cpuHasAvx512() {
   static const bool supported =
      __builtin_cpu_supports("avx512f") != 0 && __builtin_cpu_supports("avx512bw") != 0;
   return supported;
}
#endif

/// Sets bit `row` of the zeroed `bitset` for every row whose `[starts, ends)` covers `position`,
/// returning the count. Uses AVX-512 where available, scalar everywhere else.
uint32_t coverageScan(
   const uint32_t* starts,
   const uint32_t* ends,
   size_t num_rows,
   uint32_t position,
   uint64_t* bitset
) {
#ifdef SILO_HAS_X86_SIMD
   if (cpuHasAvx512()) {
      return coverageScanAvx512(starts, ends, num_rows, position, bitset);
   }
#endif
   return coverageScanScalar(starts, ends, num_rows, position, bitset);
}

}  // namespace

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
      // Vectorized scan: build the covered rows straight into a roaring bitset container (SIMD sets
      // 16 bits per instruction), then let the whole roaring compact it below. The container is
      // keyed by `chunk_id` and holds exactly the low 16 bits (row-in-chunk).
      const size_t num_rows = chunk_starts.size();
      auto* bitset = roaring::internal::bitset_container_create();
      const uint32_t cardinality =
         coverageScan(chunk_starts.data(), chunk_ends.data(), num_rows, position, bitset->words);
      if (cardinality == 0) {
         roaring::internal::bitset_container_free(bitset);
      } else {
         bitset->cardinality = static_cast<int32_t>(cardinality);
         roaring::internal::ra_append(
            &result.roaring.high_low_container, chunk_id, bitset, BITSET_CONTAINER_TYPE
         );
      }
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

   // The scan builds a dense bitset container; compact it (to a run/array) so the downstream set
   // algebra on the covered set stays cheap.
   result.runOptimize();

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
   if (chunk_id >= starts.size()) {
      return true;
   }
   return batch_max_end.at(chunk_id) <= position || batch_min_start.at(chunk_id) > position;
}

bool HorizontalCoverageIndex::positionCoveredByWholeChunk(uint32_t position, uint16_t chunk_id)
   const {
   if (chunk_id >= starts.size()) {
      return false;
   }
   // The envelope says every row's covered range includes `position` iff it lies in the
   // intersection of all those ranges, `[batch_max_start, batch_min_end)`. (A null row has range
   // [0, 0), forcing `batch_min_end` to 0, so this is always false for a chunk with nulls.)
   if (position < batch_max_start.at(chunk_id) || position >= batch_min_end.at(chunk_id)) {
      return false;
   }
   // Each row covers `position` only as the reference symbol if it records no in-region N there, so
   // reject the chunk if any of its in-region-N rows carries an N at `position`.
   const uint32_t base_row_id = static_cast<uint32_t>(chunk_id) << 16U;
   const uint64_t chunk_end_key = static_cast<uint64_t>(base_row_id) + starts[chunk_id].size();
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
