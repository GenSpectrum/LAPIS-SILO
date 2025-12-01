#pragma once

#include <map>
#include <optional>
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

      SequenceDiff()
          : container(nullptr),
            cardinality(0),
            typecode(0) {}

      SequenceDiff(
         roaring::internal::container_t* container,
         uint32_t cardinality,
         uint8_t typecode
      )
          : container(container),
            cardinality(cardinality),
            typecode(typecode) {}

      SequenceDiff(SequenceDiff&& other) noexcept
          : container(other.container),
            cardinality(other.cardinality),
            typecode(other.typecode) {
         other.container = nullptr;
      }
      SequenceDiff& operator=(SequenceDiff&& other) noexcept {
         if (this != &other) {
            std::swap(container, other.container);
            std::swap(cardinality, other.cardinality);
            std::swap(typecode, other.typecode);
         }
         return *this;
      }

      SequenceDiff(const SequenceDiff&) = delete;
      SequenceDiff& operator=(const SequenceDiff&) = delete;

      ~SequenceDiff() {
         if (container != nullptr) {
            roaring::internal::container_free(container, typecode);
         }
      }
   };
   static_assert(sizeof(SequenceDiff) == 16);

   std::map<SequenceDiffKey, SequenceDiff> vertical_bitmaps;

   using const_iterator = typename std::map<SequenceDiffKey, SequenceDiff>::const_iterator;

   void addSymbolsToPositions(
      uint32_t position_idx,
      const SymbolMap<SymbolType, std::vector<uint32_t>>& ids_per_symbol
   );

   std::pair<const_iterator, const_iterator> getRangeForPosition(uint32_t position_idx) const;

   SymbolMap<SymbolType, uint32_t> computeSymbolCountsForPosition(
      std::map<SequenceDiffKey, SequenceDiff>::const_iterator start,
      std::map<SequenceDiffKey, SequenceDiff>::const_iterator end,
      SymbolType::Symbol global_reference_symbol,
      uint32_t coverage_cardinality
   ) const;

   SymbolType::Symbol getSymbolWithHighestCount(
      const SymbolMap<SymbolType, uint32_t>& symbol_counts,
      SymbolType::Symbol global_reference_symbol
   ) const;

   std::optional<typename SymbolType::Symbol> adaptLocalReference(
      const roaring::Roaring& coverage_bitmap,
      uint32_t position_idx,
      SymbolType::Symbol global_reference_symbol
   );

   SequenceDiff& getContainerOrCreateWithCapacity(const SequenceDiffKey& key, int32_t capacity);

   roaring::Roaring getMatchingContainersAsBitmap(
      uint32_t position_idx,
      std::vector<typename SymbolType::Symbol> symbol
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
