#ifndef SILO_AA_STORE_H
#define SILO_AA_STORE_H

#include <array>
#include <deque>
#include <optional>

#include <silo/common/fasta_reader.h>
#include <spdlog/spdlog.h>
#include <boost/serialization/array.hpp>
#include <roaring/roaring.hh>

#include "silo/common/aa_symbols.h"
#include "silo/common/zstdfasta_reader.h"
#include "silo/roaring/roaring_serialize.h"
#include "silo/storage/serialize_optional.h"

namespace silo {

struct AAPosition {
   friend class boost::serialization::access;

   template <class Archive>
   void serialize(Archive& archive, [[maybe_unused]] const unsigned int version) {
      // clang-format off
      archive& symbol_whose_bitmap_is_flipped;
      archive& bitmaps;
      // clang-format on
   }

   std::array<roaring::Roaring, AA_SYMBOL_COUNT> bitmaps;
   std::optional<AA_SYMBOL> symbol_whose_bitmap_is_flipped = std::nullopt;
};

class AAStorePartition {
   friend class boost::serialization::access;

  private:
   template <class Archive>
   void serialize(Archive& archive, [[maybe_unused]] const unsigned int version) {
      // clang-format off
      archive& sequence_count;
      archive& positions;
      archive& aa_symbol_x_bitmaps;
      // clang-format on
   }

   void fillIndexes(const std::vector<std::string>& sequences);

   void fillXBitmaps(const std::vector<std::string>& sequences);

  public:
   explicit AAStorePartition(const std::string& reference_sequence);

   const std::string& reference_sequence;
   std::vector<AAPosition> positions;
   std::vector<roaring::Roaring> aa_symbol_x_bitmaps;
   unsigned sequence_count{};

   [[nodiscard]] size_t computeSize() const;

   [[nodiscard]] const roaring::Roaring* getBitmap(size_t position, AA_SYMBOL symbol) const;

   unsigned fill(silo::ZstdFastaReader& input_file);

   void interpret(const std::vector<std::string>& aa_sequences);

   unsigned runOptimize();

   unsigned shrinkToFit();
};

class AAStore {
  public:
   std::string reference_sequence;
   std::deque<AAStorePartition> partitions;

   explicit AAStore(std::string reference_sequence);

   AAStorePartition& createPartition();
};

}  // namespace silo

#endif  // SILO_AA_STORE_H
