#pragma once

#include <cstddef>
#include <cstdint>
#include <deque>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <fmt/format.h>
#include <boost/serialization/access.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/serialization/string.hpp>
#include <roaring/roaring.hh>

#include "silo/common/aa_symbols.h"
#include "silo/common/format_number.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/common/symbol_map.h"
#include "silo/common/table_reader.h"
#include "silo/storage/column/insertion_index.h"
#include "silo/storage/column/sequence_position.h"
#include "silo/storage/reference_genomes.h"

namespace silo::storage::column {

class SequenceColumnInfo {
  private:
   friend class boost::serialization::access;
   template <class Archive>
   void serialize(Archive& archive, [[maybe_unused]] const uint32_t version) {
      archive & sequence_count;
      archive & vertical_bitmaps_size;
      archive & horizontal_bitmaps_size;
   }

  public:
   uint32_t sequence_count;
   uint64_t vertical_bitmaps_size;
   uint64_t horizontal_bitmaps_size;
};

struct ReadSequence {
   bool is_valid = false;
   std::string sequence = "";
   uint32_t offset;

   ReadSequence(std::string_view _sequence, uint32_t _offset = 0)
       : is_valid(true),
         sequence(std::move(_sequence)),
         offset(_offset) {}

   ReadSequence() {}
};

template <typename SymbolType>
class SequenceColumnMetadata : public ColumnMetadata {
  public:
   std::vector<typename SymbolType::Symbol> reference_sequence;

   explicit SequenceColumnMetadata(
      std::string column_name,
      std::vector<typename SymbolType::Symbol>&& reference_sequence
   );
};

template <typename SymbolType>
class SequenceColumnPartition {
  public:
   using Metadata = SequenceColumnMetadata<SymbolType>;

   static constexpr schema::ColumnType TYPE = SymbolType::COLUMN_TYPE;

  private:
   friend class boost::serialization::access;
   template <class Archive>
   void serialize(Archive& archive, [[maybe_unused]] const uint32_t version) {
      // clang-format off
      archive & sequence_column_info;
      archive & indexing_differences_to_reference_sequence;
      archive & insertion_index;
      archive & vertical_bitmaps;
      archive & horizontal_bitmaps;
      archive & sequence_count;
      // clang-format on
   }

  public:
   // We conceptually divide the sequence space into tilings with a side length of 2^16
   //
   //        0      2^16   2*2^16 3*2^16
   //        |      |      |      |
   //      0-┌──────┬──────┬──────┬──────┬────>
   //        │      │      │      │      │  Position (x-axis)
   //        │ Tile │ Tile │ Tile │ Tile │
   //        │ 0,0  │ 0,1  │ 0,2  │ 0,3  │
   //   2^16-├──────┼──────┼──────┼──────┼
   //        │      │      │      │      │
   //        │ Tile │ Tile │ Tile │ Tile │
   //        │ 1,0  │ 1,1  │ 1,2  │ 1,3  │
   // 2*2^16-├──────┼──────┼──────┼──────┼
   //        │      │      │      │      │
   //        │ Tile │ Tile │ Tile │ Tile │
   //        │ 2,0  │ 2,1  │ 2,2  │ 2,3  │
   //        ├──────┼──────┼──────┼──────┼
   //        │
   //        v
   //    Sequence Number (y-axis)

   struct SequenceDiffKey {
      // The position in the bitmap
      uint32_t position;
      uint16_t vertical_tile_index;
      SymbolType::Symbol symbol;

      auto operator<=>(const SequenceDiffKey&) const = default;
      bool operator==(const SequenceDiffKey&) const = default;

      template <class Archive>
      void serialize(Archive& archive, [[maybe_unused]] const uint32_t version) {
         // clang-format off
         archive & position;
         archive & vertical_tile_index;
         archive & symbol;
         // clang-format on
      }
   };
   static_assert(sizeof(SequenceDiffKey) == 8);
   struct SequenceDiff {
      roaring::internal::container_t* container;
      uint16_t cardinality;
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
               auto* array = roaring::internal::array_container_create();
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

   SequenceColumnMetadata<SymbolType>* metadata;
   SequenceColumnInfo sequence_column_info;
   std::map<size_t, typename SymbolType::Symbol> indexing_differences_to_reference_sequence;
   std::map<SequenceDiffKey, SequenceDiff> vertical_bitmaps;
   std::vector<roaring::Roaring> horizontal_bitmaps;
   storage::insertion::InsertionIndex<SymbolType> insertion_index;
   uint32_t sequence_count = 0;
   size_t genome_length = 0;

   explicit SequenceColumnPartition(Metadata* metadata);

   size_t numValues() const { return sequence_count; }

   std::vector<typename SymbolType::Symbol> getLocalReference() const {
      std::vector<typename SymbolType::Symbol> local_reference = metadata->reference_sequence;
      for (auto [pos, symbol] : indexing_differences_to_reference_sequence) {
         local_reference.at(pos) = symbol;
      }
      return local_reference;
   }

   SymbolType::Symbol getLocalReferencePosition(size_t position) const {
      SILO_ASSERT(position < metadata->reference_sequence.size());
      auto iter = indexing_differences_to_reference_sequence.find(position);
      if (iter != indexing_differences_to_reference_sequence.end()) {
         return iter->second;
      }
      return metadata->reference_sequence.at(position);
   }

   [[nodiscard]] SequenceColumnInfo getInfo() const;

   ReadSequence& appendNewSequenceRead();

   void appendInsertion(const std::string& insertion_and_position);

   void finalize();

  private:
   static constexpr size_t BUFFER_SIZE = 1024;
   std::vector<ReadSequence> lazy_buffer;

   void fillIndexes();

   void addSymbolsToPositions(
      uint32_t position_idx,
      SymbolMap<SymbolType, std::vector<uint32_t>>& ids_per_symbol_for_current_position,
      size_t number_of_sequences
   );

   void fillNBitmaps();

   void optimizeBitmaps();

   void flushBuffer();

   [[nodiscard]] SequenceColumnInfo calculateInfo();

   [[nodiscard]] size_t computeVerticalBitmapsSize() const;

   [[nodiscard]] size_t computeHorizontalBitmapsSize() const;
};
}  // namespace silo::storage::column

template <>
class [[maybe_unused]] fmt::formatter<silo::storage::column::SequenceColumnInfo> {
  public:
   constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
   [[maybe_unused]] static auto format(
      const silo::storage::column::SequenceColumnInfo& sequence_store_info,
      format_context& ctx
   ) -> decltype(ctx.out());
};

BOOST_SERIALIZATION_SPLIT_FREE(silo::storage::column::SequenceColumnMetadata<silo::AminoAcid>);
namespace boost::serialization {
template <class Archive>
[[maybe_unused]] void save(
   Archive& ar,
   const silo::storage::column::SequenceColumnMetadata<silo::AminoAcid>& object,
   [[maybe_unused]] const uint32_t version
) {
   ar & object.column_name;
   ar & object.reference_sequence;
}
}  // namespace boost::serialization

BOOST_SERIALIZATION_SPLIT_FREE(std::shared_ptr<
                               silo::storage::column::SequenceColumnMetadata<silo::AminoAcid>>);
namespace boost::serialization {
template <class Archive>
[[maybe_unused]] void load(
   Archive& ar,
   std::shared_ptr<silo::storage::column::SequenceColumnMetadata<silo::AminoAcid>>& object,
   [[maybe_unused]] const uint32_t version
) {
   std::string column_name;
   std::vector<silo::AminoAcid::Symbol> reference_sequence;
   ar & column_name;
   ar & reference_sequence;
   object = std::make_shared<silo::storage::column::SequenceColumnMetadata<silo::AminoAcid>>(
      std::move(column_name), std::move(reference_sequence)
   );
}
}  // namespace boost::serialization

BOOST_SERIALIZATION_SPLIT_FREE(silo::storage::column::SequenceColumnMetadata<silo::Nucleotide>);
namespace boost::serialization {
template <class Archive>
[[maybe_unused]] void save(
   Archive& ar,
   const silo::storage::column::SequenceColumnMetadata<silo::Nucleotide>& object,
   [[maybe_unused]] const uint32_t version
) {
   ar & object.column_name;
   ar & object.reference_sequence;
}
}  // namespace boost::serialization

BOOST_SERIALIZATION_SPLIT_FREE(std::shared_ptr<
                               silo::storage::column::SequenceColumnMetadata<silo::Nucleotide>>);
namespace boost::serialization {
template <class Archive>
[[maybe_unused]] void load(
   Archive& ar,
   std::shared_ptr<silo::storage::column::SequenceColumnMetadata<silo::Nucleotide>>& object,
   [[maybe_unused]] const uint32_t version
) {
   std::string column_name;
   std::vector<silo::Nucleotide::Symbol> reference_sequence;
   ar & column_name;
   ar & reference_sequence;
   object = std::make_shared<silo::storage::column::SequenceColumnMetadata<silo::Nucleotide>>(
      std::move(column_name), std::move(reference_sequence)
   );
}
}  // namespace boost::serialization
