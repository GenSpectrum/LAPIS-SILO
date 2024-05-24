#pragma once

#include <cstddef>
#include <cstdint>
#include <deque>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <fmt/core.h>
#include <roaring/roaring.hh>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/common/symbol_map.h"
#include "silo/storage/insertion_index.h"
#include "silo/storage/position.h"

namespace boost::serialization {
class access;
}  // namespace boost::serialization

namespace silo {
template <typename SymbolType>
class Position;
class ZstdFastaTableReader;

struct SequenceStoreInfo {
   uint32_t sequence_count;
   uint64_t size;
   size_t n_bitmaps_size;
};

class ReadSequence {
  public:
   std::string_view sequence;
   uint32_t offset;

   ReadSequence(uint32_t _offset, std::string_view _sequence)
       : sequence(_sequence),
         offset(_offset) {}
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
      // clang-format on
   }

  public:
   const std::vector<typename SymbolType::Symbol>& reference_sequence;

   static constexpr size_t BUFFER_SIZE = 1024;
   std::vector<ReadSequence> lazy_buffer;

   std::vector<std::pair<size_t, typename SymbolType::Symbol>>
      indexing_differences_to_reference_sequence;
   std::vector<Position<SymbolType>> positions;
   std::vector<roaring::Roaring> missing_symbol_bitmaps;
   storage::insertion::InsertionIndex<SymbolType> insertion_index;
   uint32_t sequence_count = 0;

  private:
   void fillIndexes(const std::vector<ReadSequence>& reads);

   void addSymbolsToPositions(
      size_t position_idx,
      SymbolMap<SymbolType, std::vector<uint32_t>>& ids_per_symbol_for_current_position,
      size_t number_of_sequences
   );

   void fillNBitmaps(const std::vector<ReadSequence>& reads);

   void optimizeBitmaps();

  public:
   explicit SequenceStorePartition(
      const std::vector<typename SymbolType::Symbol>& reference_sequence
   );

   [[nodiscard]] size_t computeSize() const;

   [[nodiscard]] const roaring::Roaring* getBitmap(
      size_t position_idx,
      typename SymbolType::Symbol symbol
   ) const;

   [[nodiscard]] SequenceStoreInfo getInfo() const;

   void insertRead(size_t row_id, ReadSequence&& read);

   void insertNull(size_t row_id);

   void insertInsertion(size_t row_id, const std::string& insertion_and_position);

   void interpret(const std::vector<ReadSequence>& reads);

   void finalize();
};

template <typename SymbolType>
class SequenceStore {
  public:
   std::vector<typename SymbolType::Symbol> reference_sequence;
   std::deque<SequenceStorePartition<SymbolType>> partitions;

   explicit SequenceStore(std::vector<typename SymbolType::Symbol> reference_sequence);

   SequenceStorePartition<SymbolType>& createPartition();
};

}  // namespace silo

template <>
struct [[maybe_unused]] fmt::formatter<silo::SequenceStoreInfo> : fmt::formatter<std::string> {
   [[maybe_unused]] static auto format(
      silo::SequenceStoreInfo sequence_store_info,
      format_context& ctx
   ) -> decltype(ctx.out());
};
