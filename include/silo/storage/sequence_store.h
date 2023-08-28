
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
#include "silo/common/symbol_map.h"
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
      archive & bitmaps;
      archive & symbol_whose_bitmap_is_flipped;
      // clang-format on
   }

   NucPosition() = default;

  public:
   explicit NucPosition(Nucleotide::Symbol symbol);
   explicit NucPosition(std::optional<Nucleotide::Symbol> symbol);

   SymbolMap<Nucleotide, roaring::Roaring> bitmaps;
   std::optional<Nucleotide::Symbol> symbol_whose_bitmap_is_flipped;

   std::optional<Nucleotide::Symbol> flipMostNumerousBitmap(uint32_t sequence_count);
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
      archive & indexing_differences_to_reference_genome;
      archive & positions;
      archive & nucleotide_symbol_n_bitmaps;
      archive & sequence_count;
      // clang-format on
   }

   void fillIndexes(const std::vector<std::string>& genomes);

   void fillNBitmaps(const std::vector<std::string>& genomes);

  public:
   explicit SequenceStorePartition(const std::vector<Nucleotide::Symbol>& reference_genome);

   const std::vector<Nucleotide::Symbol>& reference_genome;
   std::vector<std::pair<size_t, Nucleotide::Symbol>> indexing_differences_to_reference_genome;
   std::vector<NucPosition> positions;
   std::vector<roaring::Roaring> nucleotide_symbol_n_bitmaps;
   uint32_t sequence_count = 0;

   [[nodiscard]] size_t computeSize() const;

   [[nodiscard]] const roaring::Roaring* getBitmap(size_t position, Nucleotide::Symbol symbol)
      const;

   [[nodiscard]] SequenceStoreInfo getInfo() const;

   size_t fill(silo::ZstdFastaReader& input_file);

   void interpret(const std::vector<std::string>& genomes);
};

class SequenceStore {
  public:
   std::vector<Nucleotide::Symbol> reference_genome;
   std::deque<SequenceStorePartition> partitions;

   explicit SequenceStore(std::vector<Nucleotide::Symbol> reference_genome);

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
