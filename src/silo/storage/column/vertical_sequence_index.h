#pragma once

#include <map>
#include <vector>

#include <boost/serialization/access.hpp>
#include <roaring/roaring.hh>

#include "silo/common/symbol_map.h"

namespace silo::storage::column {

// For documentation of this data-structure see:
// documentation/developer/sequence_storage.md
template <typename SymbolType>
class VerticalSequenceIndex {
  public:
   struct SequenceDiffKey {
      // The position in the bitmap
      uint32_t position;
      uint16_t v_index;
      SymbolType::Symbol symbol;

      auto operator<=>(const SequenceDiffKey&) const = default;
      bool operator==(const SequenceDiffKey&) const = default;

      template <class Archive>
      void serialize(Archive& archive, [[maybe_unused]] const uint32_t version) {
         // clang-format off
         archive & position;
         archive & v_index;
         archive & symbol;
         // clang-format on
      }
   };
   static_assert(sizeof(SequenceDiffKey) == 8);
   struct SequenceDiff {
      roaring::internal::container_t* container;
      uint32_t cardinality;
      uint8_t typecode;

      template <class Archive>
      void serialize(Archive& archive, [[maybe_unused]] const uint32_t version) {
         // clang-format off
         archive & cardinality;
         archive & typecode;
         // clang-format on
         if constexpr (Archive::is_saving::value) {
            size_t size_in_bytes = roaring::internal::container_size_in_bytes(container, typecode);
            std::string buffer(size_in_bytes, '\0');
            roaring::internal::container_write(container, typecode, buffer.data());
            archive << buffer;
         } else {
            std::string buffer;
            archive >> buffer;
            if (typecode == BITSET_CONTAINER_TYPE) {
               auto* bitset = roaring::internal::bitset_container_create();
               if (bitset == nullptr) {
                  throw std::runtime_error("failed to allocate bitset container");
               }
               bitset_container_read(cardinality, bitset, buffer.data());
               container = bitset;
            } else if (typecode == RUN_CONTAINER_TYPE) {
               auto* run = roaring::internal::run_container_create();
               if (run == nullptr) {
                  throw std::runtime_error("failed to allocate run container");
               }
               run_container_read(cardinality, run, buffer.data());
               container = run;
            } else {
               auto* array = roaring::internal::array_container_create_given_capacity(cardinality);
               if (array == nullptr) {
                  throw std::runtime_error("failed to allocate array container");
               }
               array_container_read(cardinality, array, buffer.data());
               container = array;
            }
         }
      }
   };
   static_assert(sizeof(SequenceDiff) == 16);

   std::map<SequenceDiffKey, SequenceDiff> vertical_bitmaps;

   void addSymbolsToPositions(
      uint32_t position_idx,
      const SymbolMap<SymbolType, std::vector<uint32_t>>& ids_per_symbol
   );

   SequenceDiff& getContainerOrCreateWithCapacity(const SequenceDiffKey& key, size_t capacity);

   roaring::Roaring getMatchingContainersAsBitmap(uint32_t position_idx, SymbolType::Symbol symbol)
      const;

   roaring::Roaring getNonMatchingContainersAsBitmap(
      uint32_t position_idx,
      SymbolType::Symbol symbol
   ) const;

   void overwriteSymbolsInSequences(
      std::vector<std::string>& sequences,
      const roaring::Roaring& row_ids
   ) const;

  private:
   friend class boost::serialization::access;
   template <class Archive>
   void serialize(Archive& archive, [[maybe_unused]] const uint32_t version) {
      archive & vertical_bitmaps;
   }
};

std::vector<std::pair<uint16_t, std::vector<uint16_t>>> splitIdsIntoBatches(
   const std::vector<uint32_t>& sorted_ids
);

}  // namespace silo::storage::column
