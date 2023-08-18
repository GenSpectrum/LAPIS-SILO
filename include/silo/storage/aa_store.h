#ifndef SILO_AA_STORE_H
#define SILO_AA_STORE_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <optional>
#include <string>
#include <vector>

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
enum class AA_SYMBOL : char;

class AAPosition {
   friend class boost::serialization::access;

   template <class Archive>
   void serialize(Archive& archive, [[maybe_unused]] const uint32_t version) {
      // clang-format off
      archive & symbol_whose_bitmap_is_flipped;
      archive & bitmaps;
      // clang-format on
   }

   AAPosition() = default;

  public:
   explicit AAPosition(AA_SYMBOL symbol);
   explicit AAPosition(std::optional<AA_SYMBOL> symbol);

   SymbolMap<AA_SYMBOL, roaring::Roaring> bitmaps;
   std::optional<AA_SYMBOL> symbol_whose_bitmap_is_flipped = std::nullopt;

   std::optional<silo::AA_SYMBOL> flipMostNumerousBitmap(uint32_t sequence_count);
};

class AAStorePartition {
   friend class boost::serialization::access;

  private:
   template <class Archive>
   void serialize(Archive& archive, [[maybe_unused]] const uint32_t version) {
      // clang-format off
      archive & sequence_count;
      archive & indexing_differences_to_reference_sequence;
      archive & positions;
      archive & aa_symbol_x_bitmaps;
      // clang-format on
   }

   void fillIndexes(const std::vector<std::string>& sequences);

   void fillXBitmaps(const std::vector<std::string>& sequences);

  public:
   explicit AAStorePartition(const std::vector<AA_SYMBOL>& reference_sequence);

   const std::vector<AA_SYMBOL>& reference_sequence;
   std::vector<std::pair<size_t, AA_SYMBOL>> indexing_differences_to_reference_sequence;
   std::vector<AAPosition> positions;
   std::vector<roaring::Roaring> aa_symbol_x_bitmaps;
   uint32_t sequence_count = 0;

   [[nodiscard]] size_t computeSize() const;

   [[nodiscard]] const roaring::Roaring* getBitmap(size_t position, AA_SYMBOL symbol) const;

   size_t fill(silo::ZstdFastaReader& input_file);

   void interpret(const std::vector<std::string>& aa_sequences);

   size_t runOptimize();

   size_t shrinkToFit();
};

class AAStore {
  public:
   std::vector<AA_SYMBOL> reference_sequence;
   std::deque<AAStorePartition> partitions;

   explicit AAStore(std::vector<AA_SYMBOL> reference_sequence);

   AAStorePartition& createPartition();
};

}  // namespace silo

#endif  // SILO_AA_STORE_H
