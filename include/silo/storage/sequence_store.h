#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <optional>
#include <string>
#include <vector>

#include <fmt/core.h>
#include <spdlog/spdlog.h>
#include <boost/serialization/array.hpp>
#include <roaring/roaring.hh>

#include "silo/common/fasta_reader.h"
#include "silo/common/symbol_map.h"
#include "silo/common/zstdfasta_reader.h"
#include "silo/roaring/roaring_serialize.h"
#include "silo/storage/serialize_optional.h"

namespace boost::serialization {
class access;
}  // namespace boost::serialization

namespace silo {
class ZstdFastaReader;

template <typename SymbolType>
class Position {
   friend class boost::serialization::access;

   template <class Archive>
   void serialize(Archive& archive, [[maybe_unused]] const uint32_t version) {
      // clang-format off
      archive & bitmaps;
      archive & symbol_whose_bitmap_is_flipped;
      // clang-format on
   }

   Position() = default;

  public:
   explicit Position(typename SymbolType::Symbol symbol);
   explicit Position(std::optional<typename SymbolType::Symbol> symbol);

   SymbolMap<SymbolType, roaring::Roaring> bitmaps;
   std::optional<typename SymbolType::Symbol> symbol_whose_bitmap_is_flipped;

   std::optional<typename SymbolType::Symbol> flipMostNumerousBitmap(uint32_t sequence_count);
};

struct SequenceStoreInfo {
   uint32_t sequence_count;
   uint64_t size;
   size_t n_bitmaps_size;
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
      archive & missing_symbol_bitmaps;
      archive & sequence_count;
      // clang-format on
   }

   void fillIndexes(const std::vector<std::string>& genomes);

   void fillNBitmaps(const std::vector<std::string>& genomes);

  public:
   explicit SequenceStorePartition(const std::vector<typename SymbolType::Symbol>& reference_genome
   );

   const std::vector<typename SymbolType::Symbol>& reference_sequence;
   std::vector<std::pair<size_t, typename SymbolType::Symbol>>
      indexing_differences_to_reference_sequence;
   std::vector<Position<SymbolType>> positions;
   std::vector<roaring::Roaring> missing_symbol_bitmaps;
   uint32_t sequence_count = 0;

   [[nodiscard]] size_t computeSize() const;

   [[nodiscard]] const roaring::Roaring* getBitmap(
      size_t position,
      typename SymbolType::Symbol symbol
   ) const;

   [[nodiscard]] SequenceStoreInfo getInfo() const;

   size_t fill(silo::ZstdFastaReader& input_file);

   void interpret(const std::vector<std::string>& genomes);
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
