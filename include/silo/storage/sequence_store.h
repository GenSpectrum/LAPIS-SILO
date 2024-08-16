#pragma once

#include <cstddef>
#include <cstdint>
#include <deque>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <fmt/format.h>
#include <duckdb/main/connection.hpp>
#include <roaring/roaring.hh>

#include "silo/common/aa_symbols.h"
#include "silo/common/format_number.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/common/symbol_map.h"
#include "silo/common/table_reader.h"
#include "silo/storage/insertion_index.h"
#include "silo/storage/position.h"

namespace boost::serialization {
class access;
}  // namespace boost::serialization

namespace silo {
template <typename SymbolType>
class Position;
class ZstdTableReader;

struct SequenceStoreInfo {
   uint32_t sequence_count;
   uint64_t size;
   size_t n_bitmaps_size;
};

struct ReadSequence {
   bool is_valid = false;
   std::string sequence = "";
   uint32_t offset;

   ReadSequence(std::string_view _sequence, uint32_t _offset = 0)
       : sequence(std::move(_sequence)),
         offset(_offset),
         is_valid(true) {}

   ReadSequence() {}
};

template <typename SymbolType>
class SequenceStorePartition {
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
      archive & sparse_mode;
      // clang-format on
   }

  public:
   const std::vector<typename SymbolType::Symbol>& reference_sequence;
   std::vector<std::pair<size_t, typename SymbolType::Symbol>>
      indexing_differences_to_reference_sequence;
   std::vector<Position<SymbolType>> positions;
   std::vector<roaring::Roaring> missing_symbol_bitmaps;
   storage::insertion::InsertionIndex<SymbolType> insertion_index;
   uint32_t sequence_count = 0;
   bool sparse_mode = false;

  private:
   static constexpr size_t BUFFER_SIZE = 1024;
   std::vector<ReadSequence> lazy_buffer;

   void fillIndexes(const std::vector<ReadSequence>& reads);

   void addSymbolsToPositions(
      size_t position_idx,
      SymbolMap<SymbolType, std::vector<uint32_t>>& ids_per_symbol_for_current_position,
      size_t number_of_sequences
   );

   void fillNBitmaps(const std::vector<ReadSequence>& reads);

   void optimizeBitmaps();

   void flushBuffer(const std::vector<ReadSequence>& reads);

  public:
   explicit SequenceStorePartition(
      const std::vector<typename SymbolType::Symbol>& reference_sequence, bool sparse_mode
   );

   [[nodiscard]] size_t computeSize() const;

   [[nodiscard]] const roaring::Roaring* getBitmap(
      size_t position_idx,
      typename SymbolType::Symbol symbol
   ) const;

   [[nodiscard]] SequenceStoreInfo getInfo() const;

   ReadSequence& appendNewSequenceRead();

   void insertInsertion(size_t row_id, const std::string& insertion_and_position);

   void finalize();
};

template <typename SymbolType>
class SequenceStore {
  public:
   std::vector<typename SymbolType::Symbol> reference_sequence;
   std::deque<SequenceStorePartition<SymbolType>> partitions;
   bool sparse_mode = false;

   explicit SequenceStore(std::vector<typename SymbolType::Symbol> reference_sequence, bool sparse_mode);

   SequenceStorePartition<SymbolType>& createPartition();
};

}  // namespace silo

template <>
class [[maybe_unused]] fmt::formatter<silo::SequenceStoreInfo> {
  public:
   constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
   [[maybe_unused]] static auto format(
      const silo::SequenceStoreInfo& sequence_store_info,
      format_context& ctx
   ) -> decltype(ctx.out());
};