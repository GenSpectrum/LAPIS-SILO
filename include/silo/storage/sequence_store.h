
#ifndef SILO_SEQUENCE_STORE_H
#define SILO_SEQUENCE_STORE_H

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
#include "silo/common/nucleotide_symbol_map.h"
#include "silo/common/zstdfasta_reader.h"
#include "silo/roaring/roaring_serialize.h"
#include "silo/storage/serialize_optional.h"

namespace boost::serialization {
class access;
}  // namespace boost::serialization

namespace silo {
class ZstdFastaReader;

class NucPosition {
   friend class boost::serialization::access;

   template <class Archive>
   void serialize(Archive& archive, [[maybe_unused]] const uint32_t version) {
      // clang-format off
      archive & symbol_whose_bitmap_is_flipped;
      archive & bitmaps;
      // clang-format on
   }

   NucPosition() = default;

  public:
   explicit NucPosition(NUCLEOTIDE_SYMBOL symbol);
   explicit NucPosition(std::optional<NUCLEOTIDE_SYMBOL> symbol);

   NucleotideSymbolMap<roaring::Roaring> bitmaps;
   std::optional<NUCLEOTIDE_SYMBOL> symbol_whose_bitmap_is_flipped = std::nullopt;

   std::optional<silo::NUCLEOTIDE_SYMBOL> flipMostNumerousBitmap(uint32_t sequence_count);
};

struct SequenceStoreInfo {
   uint32_t sequence_count;
   uint64_t size;
   size_t n_bitmaps_size;
};

class SequenceStorePartition {
   friend class boost::serialization::access;

   template <class Archive>
   void serialize(Archive& archive, [[maybe_unused]] const uint32_t version) {
      // clang-format off
      archive & positions;
      archive & indexing_differences_to_reference_genome;
      archive & nucleotide_symbol_n_bitmaps;
      archive & sequence_count;
      // clang-format on
   }

   void fillIndexes(const std::vector<std::string>& genomes);

   void fillNBitmaps(const std::vector<std::string>& genomes);

  public:
   explicit SequenceStorePartition(const std::vector<NUCLEOTIDE_SYMBOL>& reference_genome);

   const std::vector<NUCLEOTIDE_SYMBOL>& reference_genome;
   std::vector<std::pair<size_t, NUCLEOTIDE_SYMBOL>> indexing_differences_to_reference_genome;
   std::vector<NucPosition> positions;
   std::vector<roaring::Roaring> nucleotide_symbol_n_bitmaps;
   uint32_t sequence_count = 0;

   [[nodiscard]] size_t computeSize() const;

   [[nodiscard]] const roaring::Roaring* getBitmap(size_t position, NUCLEOTIDE_SYMBOL symbol) const;

   [[nodiscard]] SequenceStoreInfo getInfo() const;

   size_t fill(silo::ZstdFastaReader& input_file);

   void interpret(const std::vector<std::string>& genomes);

   size_t runOptimize();

   size_t shrinkToFit();
};

class SequenceStore {
  public:
   std::vector<NUCLEOTIDE_SYMBOL> reference_genome;
   std::deque<SequenceStorePartition> partitions;

   explicit SequenceStore(std::vector<NUCLEOTIDE_SYMBOL> reference_genome);

   SequenceStorePartition& createPartition();
};

}  // namespace silo

template <>
struct [[maybe_unused]] fmt::formatter<silo::SequenceStoreInfo> : fmt::formatter<std::string> {
   [[maybe_unused]] static auto format(
      silo::SequenceStoreInfo sequence_store_info,
      format_context& ctx
   ) -> decltype(ctx.out());
};

#endif  // SILO_SEQUENCE_STORE_H
