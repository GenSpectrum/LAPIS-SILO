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
      for(auto& position : positions){
            archive & position;
      }
      archive & insertion_index;
      archive & missing_symbol_bitmaps;
      archive & sequence_count;
      // clang-format on
   }

  public:
   SequenceColumnMetadata<SymbolType>* metadata;
   SequenceColumnInfo sequence_column_info;
   std::vector<std::pair<size_t, typename SymbolType::Symbol>>
      indexing_differences_to_reference_sequence;
   std::vector<SequencePosition<SymbolType>> positions;
   std::vector<roaring::Roaring> missing_symbol_bitmaps;
   storage::insertion::InsertionIndex<SymbolType> insertion_index;
   uint32_t sequence_count = 0;

   explicit SequenceColumnPartition(Metadata* metadata);

   [[nodiscard]] const roaring::Roaring* getBitmap(
      size_t position_idx,
      typename SymbolType::Symbol symbol
   ) const;

   [[nodiscard]] SequenceColumnInfo getInfo() const;

   ReadSequence& appendNewSequenceRead();

   void appendInsertion(const std::string& insertion_and_position);

   void finalize();

  private:
   static constexpr size_t BUFFER_SIZE = 1024;
   std::vector<ReadSequence> lazy_buffer;

   void fillIndexes();

   void addSymbolsToPositions(
      size_t position_idx,
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
