#pragma once

#include <algorithm>
#include <cstdint>
#include <map>
#include <utility>
#include <vector>

#include <boost/serialization/access.hpp>
#include <roaring/roaring.hh>

#include "silo/roaring_util/bitmap_builder.h"
#include "silo/storage/column/row_id.h"

namespace silo {
class Coverage;
}

namespace silo::storage::column {

class HorizontalCoverageIndex {
  public:
   /// Per-row N positions inside the covered region, keyed by sparse global row id (chunk `k` lives
   /// at `k << 16`, see `RowId`). Only rows that actually carry such positions get an entry.
   std::map<uint32_t, roaring::Roaring> horizontal_bitmaps;

   /// Per-chunk covered range of every row: `start_end[chunk_id][row_in_chunk]` is the `[start,
   /// end)` of the row whose global id is `(chunk_id << 16) | row_in_chunk`. Stored chunk by chunk
   /// (rather than as one flat dense vector) so it matches the 2^16-aligned `RowId` layout and the
   /// gaps between partially-filled chunks cost nothing.
   std::vector<std::vector<std::pair<uint32_t, uint32_t>>> start_end;

   // Also store the [start, end) range of each 2^16-aligned chunk of sequences. This allows faster
   // computations as whole chunks can be skipped if they cannot have coverage at a given position.
   std::vector<std::pair<uint32_t, uint32_t>> batch_start_ends;

   void insertCoverage(RowId row_id, const Coverage& coverage);

   void insertNullSequence(RowId row_id);

   /// The number of rows covering each position
   [[nodiscard]] std::vector<uint64_t> computeCoverageCardinalities(size_t genome_length) const;

   [[nodiscard]] size_t numChunks() const { return start_end.size(); }

   [[nodiscard]] uint32_t chunkSize(uint16_t chunk_id) const {
      return static_cast<uint32_t>(start_end.at(chunk_id).size());
   }

   /// The covered `[start, end)` range of the row addressed by its sparse global row id.
   [[nodiscard]] std::pair<uint32_t, uint32_t> coverageRange(uint32_t global_row_id) const {
      const RowId row_id = RowId::fromGlobal(global_row_id);
      return start_end.at(row_id.chunk_id).at(row_id.row_in_chunk);
   }

   template <size_t BatchSize>
   [[nodiscard]] std::array<roaring::Roaring, BatchSize> getCoverageBitmapForPositions(
      uint32_t position
   ) const {
      const uint32_t range_start = position;
      const uint32_t range_end = position + BatchSize;

      using roaring_util::BitmapBuilderByRange;
      std::array<BitmapBuilderByRange, BatchSize> result_builders;

      for (size_t chunk_id = 0; chunk_id < start_end.size(); ++chunk_id) {
         auto [batch_start, batch_end] = batch_start_ends.at(chunk_id);
         if (batch_end <= range_start || batch_start >= range_end) {
            continue;
         }
         const uint32_t base_row_id = static_cast<uint32_t>(chunk_id) << 16;
         const auto& chunk = start_end[chunk_id];
         for (size_t row_in_chunk = 0; row_in_chunk < chunk.size(); ++row_in_chunk) {
            const uint32_t row_id = base_row_id | static_cast<uint32_t>(row_in_chunk);
            auto [coverage_start, coverage_end] = chunk[row_in_chunk];
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
