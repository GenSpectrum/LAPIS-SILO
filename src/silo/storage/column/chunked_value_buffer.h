#pragma once

#include <algorithm>
#include <vector>

#include <boost/serialization/access.hpp>
#include <boost/serialization/vector.hpp>

namespace silo::storage::column {

/// Stores a value column's data as the list of fixed-width buffers in which it was ingested, one
/// buffer per appended chunk. Appending never touches previously ingested chunks, and a row id is
/// mapped to its chunk with a binary search over `chunk_end_offsets`.
template <typename T>
class ChunkedValueBuffer {
   std::vector<std::vector<T>> chunks;
   /// Running row count after each chunk; `chunk_end_offsets.back()` is the total value count.
   std::vector<size_t> chunk_end_offsets;

  public:
   void appendChunk(std::vector<T>&& values) {
      const size_t total = numValues() + values.size();
      chunks.push_back(std::move(values));
      chunk_end_offsets.push_back(total);
   }

   [[nodiscard]] size_t numValues() const {
      return chunk_end_offsets.empty() ? 0 : chunk_end_offsets.back();
   }

   [[nodiscard]] size_t numChunks() const { return chunks.size(); }

   /// The raw value buffer of chunk `chunk_idx`. Exposed so a sorted column can be binary searched
   /// in place (see `DateBetween`).
   [[nodiscard]] const std::vector<T>& chunk(size_t chunk_idx) const {
      return chunks.at(chunk_idx);
   }

   [[nodiscard]] size_t chunkStart(size_t chunk_idx) const {
      return chunk_idx == 0 ? 0 : chunk_end_offsets[chunk_idx - 1];
   }

   [[nodiscard]] const T& at(size_t row_id) const {
      // The chunk containing `row_id` is the first whose (exclusive) end offset exceeds it.
      const size_t chunk_idx = static_cast<size_t>(
         std::ranges::upper_bound(chunk_end_offsets, row_id) - chunk_end_offsets.begin()
      );
      return chunks.at(chunk_idx).at(row_id - chunkStart(chunk_idx));
   }

  private:
   friend class boost::serialization::access;
   template <class Archive>
   void serialize(Archive& archive, const uint32_t /* version */) {
      archive & chunks;
      // `chunk_end_offsets` is a derived index, not serialized; rebuild it from the loaded chunks.
      if constexpr (Archive::is_loading::value) {
         chunk_end_offsets.clear();
         chunk_end_offsets.reserve(chunks.size());
         size_t total = 0;
         for (const auto& chunk_values : chunks) {
            total += chunk_values.size();
            chunk_end_offsets.push_back(total);
         }
      }
   }
};

}  // namespace silo::storage::column
