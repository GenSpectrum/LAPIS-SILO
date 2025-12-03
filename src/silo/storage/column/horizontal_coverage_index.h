#pragma once

#include <cstddef>
#include <map>
#include <vector>

#include <boost/serialization/access.hpp>
#include <roaring/roaring.hh>

#include "silo/common/panic.h"
#include "silo/roaring_util/bitmap_builder.h"

namespace silo::storage::column {

class HorizontalCoverageIndex {
  public:
   std::map<uint32_t, roaring::Roaring> horizontal_bitmaps;
   std::vector<std::pair<uint32_t, uint32_t>> start_end;

   // Also store the [start, end) range of 2^16 size batches of sequences.
   // This allows faster computations as many sequences can be skipped
   // if we can be sure they have no coverage at given positions.
   std::vector<std::pair<uint32_t, uint32_t>> batch_start_ends;

   void insertCoverage(
      uint32_t start,
      uint32_t end,
      const std::vector<uint32_t>& positions_with_symbol_missing
   );

   void insertNullSequence();

   template <typename SymbolType>
   void insertSequenceCoverage(std::string sequence, uint32_t offset);

   template <size_t BatchSize>
   [[nodiscard]] std::array<roaring::Roaring, BatchSize> getCoverageBitmapForPositions(
      uint32_t position
   ) const {
      size_t row_count = start_end.size();

      uint32_t range_start = position;
      uint32_t range_end = position + BatchSize;

      using silo::roaring_util::BitmapBuilderByRange;
      std::array<BitmapBuilderByRange, BatchSize> result_builders;

      for (uint32_t row_id_upper_bits = 0; row_id_upper_bits << 16 < row_count;
           ++row_id_upper_bits) {
         uint32_t base_row_id = row_id_upper_bits << 16;
         SILO_ASSERT(batch_start_ends.size() > row_id_upper_bits);
         auto [batch_start, batch_end] = batch_start_ends.at(row_id_upper_bits);
         if (batch_end <= range_start || batch_start >= range_end) {
            continue;
         }
         for (uint32_t row_id_lower_bits = 0;
              row_id_lower_bits <= 0xFFFF && base_row_id + row_id_lower_bits < row_count;
              ++row_id_lower_bits) {
            uint32_t row_id = base_row_id | row_id_lower_bits;
            auto [coverage_start, coverage_end] = start_end.at(row_id);
            for (uint32_t pos = std::max(range_start, coverage_start);
                 pos < std::min(range_end, coverage_end);
                 ++pos) {
               result_builders[pos - range_start].add(row_id);
            }
         }
      }

      std::array<roaring::Roaring, BatchSize> result;
      std::ranges::transform(result_builders, result.begin(), [](BitmapBuilderByRange& builder) {
         return std::move(builder).getBitmap();
      });

      roaring::Roaring range_bitmap;
      range_bitmap.addRange(range_start, range_end);
      for (const auto& [sequence_idx, bitmap] : horizontal_bitmaps) {
         auto bitmap_in_range = bitmap & range_bitmap;
         for (auto pos : bitmap_in_range) {
            result[pos - range_start].remove(sequence_idx);
         }
      }
      return result;
   }

   template <typename SymbolType>
   void overwriteCoverageInSequence(
      std::vector<std::string>& sequences,
      const roaring::Roaring& row_ids
   ) const;

  private:
   friend class boost::serialization::access;
   template <class Archive>
   void serialize(Archive& archive, [[maybe_unused]] const uint32_t version) {
      archive & horizontal_bitmaps;
      archive & start_end;
      archive & batch_start_ends;
   }
};

}  // namespace silo::storage::column
