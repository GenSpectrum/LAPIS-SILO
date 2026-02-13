#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include <fmt/format.h>
#include <boost/serialization/access.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/serialization/string.hpp>
#include <roaring/roaring.hh>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/storage/column/horizontal_coverage_index.h"
#include "silo/storage/column/insertion_index.h"
#include "silo/storage/column/vertical_sequence_index.h"

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
      archive & local_reference_sequence_string;
      archive & vertical_sequence_index;
      archive & horizontal_coverage_index;
      archive & insertion_index;
      archive & sequence_column_info;
      archive & sequence_count;
      // clang-format on
   }

  public:
   const SequenceColumnMetadata<SymbolType>* metadata;
   const size_t genome_length;

   // Store the local reference sequence as a string to speed up insertions
   std::string local_reference_sequence_string;

   VerticalSequenceIndex<SymbolType> vertical_sequence_index;
   HorizontalCoverageIndex horizontal_coverage_index;
   storage::insertion::InsertionIndex<SymbolType> insertion_index;
   SequenceColumnInfo sequence_column_info;
   uint32_t sequence_count = 0;

   explicit SequenceColumnPartition(Metadata* metadata);

   [[nodiscard]] size_t numValues() const { return sequence_count; }

   [[nodiscard]] std::vector<typename SymbolType::Symbol> getLocalReference() const {
      std::vector<typename SymbolType::Symbol> local_reference;
      for (const char character : local_reference_sequence_string) {
         local_reference.push_back(SymbolType::charToSymbol(character).value());
      };
      return local_reference;
   }

   [[nodiscard]] SymbolType::Symbol getLocalReferencePosition(size_t position) const {
      SILO_ASSERT(position < metadata->reference_sequence.size());
      return SymbolType::charToSymbol(local_reference_sequence_string.at(position)).value();
   }

   [[nodiscard]] SequenceColumnInfo getInfo() const;

   void append(
      std::string_view sequence,
      uint32_t offset,
      const std::vector<std::string>& insertions
   );

   void appendNull();

   void finalize();

  private:
   static constexpr size_t BUFFER_SIZE = 1024;
   std::vector<SymbolMap<SymbolType, std::vector<uint32_t>>> mutation_buffer;

   void fillIndexes();

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
   static constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
   [[maybe_unused]] static auto format(
      const silo::storage::column::SequenceColumnInfo& sequence_store_info,
      format_context& ctx
   ) -> decltype(ctx.out());
};

BOOST_SERIALIZATION_SPLIT_FREE(silo::storage::column::SequenceColumnMetadata<silo::AminoAcid>);
namespace boost::serialization {
template <class Archive>
[[maybe_unused]] void save(
   Archive& archive,
   const silo::storage::column::SequenceColumnMetadata<silo::AminoAcid>& object,
   [[maybe_unused]] const uint32_t version
) {
   archive & object.column_name;
   archive & object.reference_sequence;
}
}  // namespace boost::serialization

BOOST_SERIALIZATION_SPLIT_FREE(std::shared_ptr<
                               silo::storage::column::SequenceColumnMetadata<silo::AminoAcid>>);
namespace boost::serialization {
template <class Archive>
[[maybe_unused]] void load(
   Archive& archive,
   std::shared_ptr<silo::storage::column::SequenceColumnMetadata<silo::AminoAcid>>& object,
   [[maybe_unused]] const uint32_t version
) {
   std::string column_name;
   std::vector<silo::AminoAcid::Symbol> reference_sequence;
   archive & column_name;
   archive & reference_sequence;
   object = std::make_shared<silo::storage::column::SequenceColumnMetadata<silo::AminoAcid>>(
      std::move(column_name), std::move(reference_sequence)
   );
}
}  // namespace boost::serialization

BOOST_SERIALIZATION_SPLIT_FREE(silo::storage::column::SequenceColumnMetadata<silo::Nucleotide>);
namespace boost::serialization {
template <class Archive>
[[maybe_unused]] void save(
   Archive& archive,
   const silo::storage::column::SequenceColumnMetadata<silo::Nucleotide>& object,
   [[maybe_unused]] const uint32_t version
) {
   archive & object.column_name;
   archive & object.reference_sequence;
}
}  // namespace boost::serialization

BOOST_SERIALIZATION_SPLIT_FREE(std::shared_ptr<
                               silo::storage::column::SequenceColumnMetadata<silo::Nucleotide>>);
namespace boost::serialization {
template <class Archive>
[[maybe_unused]] void load(
   Archive& archive,
   std::shared_ptr<silo::storage::column::SequenceColumnMetadata<silo::Nucleotide>>& object,
   [[maybe_unused]] const uint32_t version
) {
   std::string column_name;
   std::vector<silo::Nucleotide::Symbol> reference_sequence;
   archive & column_name;
   archive & reference_sequence;
   object = std::make_shared<silo::storage::column::SequenceColumnMetadata<silo::Nucleotide>>(
      std::move(column_name), std::move(reference_sequence)
   );
}
}  // namespace boost::serialization
