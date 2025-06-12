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
#include <roaring/roaring.hh>

#include "silo/common/aa_symbols.h"
#include "silo/common/format_number.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/common/symbol_map.h"
#include "silo/common/table_reader.h"
#include "silo/storage/column/insertion_index.h"
#include "silo/storage/column/position.h"
#include "silo/storage/reference_genomes.h"

namespace silo::storage::column {

struct SequenceColumnInfo {
   uint32_t sequence_count;
   uint64_t size;
   size_t n_bitmaps_size;
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

   YAML::Node toYAML() const {
      YAML::Node yaml_node;
      yaml_node["referenceSequence"] =
         ReferenceGenomes::vectorToString<SymbolType>(reference_sequence);
      return yaml_node;
   }

   static std::shared_ptr<SequenceColumnMetadata<SymbolType>> fromYAML(
      std::string column_name,
      const YAML::Node& yaml_node
   ) {
      std::string reference_sequence_string = yaml_node["referenceSequence"].as<std::string>();
      return std::make_shared<SequenceColumnMetadata<SymbolType>>(
         std::move(column_name),
         silo::ReferenceGenomes::stringToVector<SymbolType>(reference_sequence_string)
      );
   }
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
   std::vector<std::pair<size_t, typename SymbolType::Symbol>>
      indexing_differences_to_reference_sequence;
   std::vector<Position<SymbolType>> positions;
   std::vector<roaring::Roaring> missing_symbol_bitmaps;
   storage::insertion::InsertionIndex<SymbolType> insertion_index;
   uint32_t sequence_count = 0;

   explicit SequenceColumnPartition(Metadata* metadata);

   [[nodiscard]] size_t computeSize() const;

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