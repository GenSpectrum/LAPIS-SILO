#include "silo/storage/column/vertical_sequence_index.h"

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/common/panic.h"
#include "silo/roaring_util/subset_ranks.h"

namespace silo::storage::column {

template <typename SymbolType>
void VerticalSequenceIndex<SymbolType>::addSymbolsToPositions(
   uint32_t position_idx,
   const SymbolMap<SymbolType, std::vector<uint32_t>>& ids_per_symbol
) {
   for (const auto& symbol : SymbolType::SYMBOLS) {
      std::vector<std::pair<uint16_t, std::vector<uint16_t>>> ids_in_batches =
         splitIdsIntoBatches(ids_per_symbol.at(symbol));
      for (const auto& [upper_bits, lower_bits_vector] : ids_in_batches) {
         SILO_ASSERT_GT(lower_bits_vector.size(), 0);

         SequenceDiffKey key{.position = position_idx, .v_index = upper_bits, .symbol = symbol};
         SequenceDiff& sequence_diff =
            getContainerOrCreateWithCapacity(key, lower_bits_vector.size());

         for (auto id : lower_bits_vector) {
            uint8_t new_container_type;
            auto new_container = roaring::internal::container_add(
               sequence_diff.container, id, sequence_diff.typecode, &new_container_type
            );
            if (new_container != sequence_diff.container) {
               roaring::internal::container_free(sequence_diff.container, sequence_diff.typecode);
               sequence_diff.container = new_container;
               sequence_diff.typecode = new_container_type;
            }
            sequence_diff.cardinality += 1;
         }
      }
   }
}

template <typename SymbolType>
VerticalSequenceIndex<SymbolType>::SequenceDiff& VerticalSequenceIndex<
   SymbolType>::getContainerOrCreateWithCapacity(const SequenceDiffKey& key, size_t capacity) {
   auto iter = vertical_bitmaps.find(key);
   if (iter != vertical_bitmaps.end()) {
      return iter->second;
   } else {
      roaring::internal::container_t* container;
      uint8_t typecode;
      // If roaring::internal::DEFAULT_MAX_SIZE specifies the maximum size for array containers
      if (capacity <= roaring::internal::DEFAULT_MAX_SIZE) {
         container = roaring::internal::array_container_create_given_capacity(capacity);
         typecode = ARRAY_CONTAINER_TYPE;
      } else {
         container = roaring::internal::bitset_container_create();
         typecode = BITSET_CONTAINER_TYPE;
      }
      SILO_ASSERT(container != nullptr);
      return vertical_bitmaps
         .insert({key, SequenceDiff{.container = container, .cardinality = 0, .typecode = typecode}}
         )
         .first->second;
   }
}

template <typename SymbolType>
roaring::Roaring VerticalSequenceIndex<SymbolType>::getMatchingContainersAsBitmap(
   uint32_t position_idx,
   SymbolType::Symbol symbol
) const {
   // We compute the range of bitmap containers that are relevant for our position
   auto start = vertical_bitmaps.lower_bound(
      SequenceDiffKey{position_idx, 0, static_cast<SymbolType::Symbol>(0)}
   );
   auto end = vertical_bitmaps.lower_bound(
      SequenceDiffKey{position_idx + 1, 0, static_cast<SymbolType::Symbol>(0)}
   );
   // We construct a roaring bitmap of all bitmap containers that are of my symbol
   roaring::Roaring bitmap;
   for (auto it = start; it != end; ++it) {
      const auto& [sequence_diff_key, sequence_diff] = *it;
      if (sequence_diff_key.symbol == symbol) {
         uint8_t result_typecode = sequence_diff.typecode;
         roaring::internal::container_t* result_container =
            roaring::internal::container_clone(sequence_diff.container, sequence_diff.typecode);
         roaring::internal::ra_append(
            &bitmap.roaring.high_low_container,
            sequence_diff_key.v_index,
            result_container,
            result_typecode
         );
      }
   }
   return bitmap;
}

template <typename SymbolType>
roaring::Roaring VerticalSequenceIndex<SymbolType>::getNonMatchingContainersAsBitmap(
   uint32_t position_idx,
   SymbolType::Symbol symbol
) const {
   // We compute the range of bitmap containers that are relevant for our position
   auto start = vertical_bitmaps.lower_bound(
      SequenceDiffKey{position_idx, 0, static_cast<SymbolType::Symbol>(0)}
   );
   auto end = vertical_bitmaps.lower_bound(
      SequenceDiffKey{position_idx + 1, 0, static_cast<SymbolType::Symbol>(0)}
   );
   // We need to union all bitmap containers at this position
   roaring::Roaring bitmap;
   int32_t current_v_tile_index = -1;
   roaring::internal::container_t* current_container = nullptr;
   uint8_t current_typecode = 0;
   for (auto it = start; it != end; ++it) {
      const auto& [sequence_diff_key, sequence_diff] = *it;
      SILO_ASSERT(sequence_diff.cardinality > 0);

      SILO_ASSERT(current_v_tile_index <= sequence_diff_key.v_index);
      if (current_v_tile_index != sequence_diff_key.v_index) {
         if (current_container != nullptr) {
            roaring::internal::ra_append(
               &bitmap.roaring.high_low_container,
               current_v_tile_index,
               current_container,
               current_typecode
            );
         }
         current_v_tile_index = sequence_diff_key.v_index;
         current_typecode = sequence_diff.typecode;
         current_container =
            roaring::internal::container_clone(sequence_diff.container, sequence_diff.typecode);
      } else { /* current_v_tile_index == sequence_diff_key.v_index */
         SILO_ASSERT(current_container != nullptr);
         uint8_t result_typecode;
         roaring::internal::container_t* result_container = roaring::internal::container_ior(
            current_container,
            current_typecode,
            sequence_diff.container,
            sequence_diff.typecode,
            &result_typecode
         );
         if (result_container != current_container) {
            roaring::internal::container_free(current_container, current_typecode);
            current_container = result_container;
            current_typecode = result_typecode;
         }
      }
   }

   if (current_container != nullptr) {
      roaring::internal::ra_append(
         &bitmap.roaring.high_low_container,
         current_v_tile_index,
         current_container,
         current_typecode
      );
   }
   return bitmap;
}

using silo::roaring_util::roaringSubsetRanks;

template <typename SymbolType>
void VerticalSequenceIndex<SymbolType>::overwriteSymbolsInSequences(
   std::vector<std::string>& sequences,
   const roaring::Roaring& row_ids
) const {
   SILO_ASSERT_EQ(sequences.size(), row_ids.cardinality());
   if (row_ids.roaring.high_low_container.size == 0) {
      return;
   }
   size_t max_v_index =
      row_ids.roaring.high_low_container.keys[row_ids.roaring.high_low_container.size - 1];

   // Construct the slicesarrays that correspond to individual roaring containers = v_index
   std::vector<std::string*> sequences_by_v_index(max_v_index + 1);
   std::vector<size_t> sequences_by_v_index_sizes(max_v_index + 1);
   std::vector<roaring::internal::container_t*> roaring_containers_by_v_index(max_v_index + 1);
   std::vector<uint8_t> roaring_typecodes_by_v_index(max_v_index + 1);
   std::string* current_sequences_pointer = sequences.data();
   for (size_t idx = 0; idx < row_ids.roaring.high_low_container.size; ++idx) {
      auto cardinality = roaring::internal::container_get_cardinality(
         row_ids.roaring.high_low_container.containers[idx],
         row_ids.roaring.high_low_container.typecodes[idx]
      );
      auto key = row_ids.roaring.high_low_container.keys[idx];
      SILO_ASSERT(key <= max_v_index);
      sequences_by_v_index.at(key) = current_sequences_pointer;
      sequences_by_v_index_sizes.at(key) = cardinality;
      roaring_containers_by_v_index.at(key) = row_ids.roaring.high_low_container.containers[idx];
      roaring_typecodes_by_v_index.at(key) = row_ids.roaring.high_low_container.typecodes[idx];
      current_sequences_pointer += cardinality;
   }

   for (const auto& [sequence_diff_key, sequence_diff] : vertical_bitmaps) {
      const uint16_t v_index = sequence_diff_key.v_index;

      if (v_index > max_v_index || sequences_by_v_index_sizes.at(v_index) == 0) {
         continue;
      }

      auto ranks_in_reconstructed_sequences = roaringSubsetRanks(
         roaring_containers_by_v_index.at(v_index),
         roaring_typecodes_by_v_index.at(v_index),
         sequence_diff.container,
         sequence_diff.typecode,
         /*base=*/0  // Base rank is 0, because we have a slice per container
      );

      // Ranks are ordered, back() = largest rank -> should be a valid size for the current sequence
      // slice
      SILO_ASSERT(
         ranks_in_reconstructed_sequences.empty() ||
         ranks_in_reconstructed_sequences.back() <= sequences_by_v_index_sizes.at(v_index)
      );

      auto& current_v_index_sequences = sequences_by_v_index.at(v_index);
      for (auto rank_in_reconstructed_sequences : ranks_in_reconstructed_sequences) {
         // Ranks are 1-indexed
         uint32_t id_in_reconstructed_sequences = rank_in_reconstructed_sequences - 1;
         current_v_index_sequences[id_in_reconstructed_sequences].at(sequence_diff_key.position) =
            SymbolType::symbolToChar(sequence_diff_key.symbol);
      }
   }
};

template class VerticalSequenceIndex<Nucleotide>;
template class VerticalSequenceIndex<AminoAcid>;

std::vector<std::pair<uint16_t, std::vector<uint16_t>>> splitIdsIntoBatches(
   const std::vector<uint32_t>& sorted_ids
) {
   std::vector<std::pair<uint16_t, std::vector<uint16_t>>> ids_in_batches;

   if (sorted_ids.empty()) {
      return ids_in_batches;
   }

   uint16_t current_upper_bits = sorted_ids.front() >> 16;
   ids_in_batches.push_back(std::make_pair<uint16_t, std::vector<uint16_t>>(
      std::move(current_upper_bits), std::vector<uint16_t>{}
   ));

   for (uint32_t id : sorted_ids) {
      uint16_t upper_bits = id >> 16;
      uint16_t lower_bits = id & 0xFFFF;

      // If the upper bits changed, start a new batch
      if (upper_bits != current_upper_bits) {
         current_upper_bits = upper_bits;
         ids_in_batches.emplace_back(upper_bits, std::vector<uint16_t>{});
      }

      // Add the lower bits to the current batch
      ids_in_batches.back().second.push_back(lower_bits);
   }

   return ids_in_batches;
}

}  // namespace silo::storage::column
