
#ifndef SILO_SEQUENCE_STORE_H
#define SILO_SEQUENCE_STORE_H

#include <spdlog/spdlog.h>
#include <array>
#include <boost/serialization/array.hpp>
#include <roaring/roaring.hh>

#include "metadata_store.h"
#include "silo/roaring/roaring_serialize.h"

namespace silo {

struct Position {
   friend class boost::serialization::access;

   template <class Archive>
   void serialize(Archive& archive, [[maybe_unused]] const unsigned int version) {
      archive& flipped_bitmap;
      archive& bitmaps;
      archive& nucleotide_symbol_n_indexed;
   }

   std::array<roaring::Roaring, SYMBOL_COUNT> bitmaps;

   static constexpr uint32_t REFERENCE_BITMAP_IS_FLIPPED = UINT32_MAX;
   uint32_t flipped_bitmap = REFERENCE_BITMAP_IS_FLIPPED;

   bool nucleotide_symbol_n_indexed = false;
};

struct SequenceStoreInfo {
   uint32_t sequence_count;
   uint64_t size;
   size_t n_bitmaps_size;
};

class SequenceStore {
  private:
   unsigned sequence_count;

  public:
   friend class boost::serialization::access;

   template <class Archive>
   void serialize(Archive& archive, [[maybe_unused]] const unsigned int version) {
      archive& sequence_count;
      archive& positions;
      archive& nucleotide_symbol_n_bitmaps;
   }

   std::array<Position, GENOME_LENGTH> positions;
   std::vector<roaring::Roaring> nucleotide_symbol_n_bitmaps;

   SequenceStore();

   [[nodiscard]] size_t computeSize() const;

   [[nodiscard]] const roaring::Roaring* getBitmap(size_t position, GENOME_SYMBOL symbol) const;

   [[nodiscard]] roaring::Roaring* getBitmapFromAmbiguousSymbol(
      size_t position,
      GENOME_SYMBOL ambiguous_symbol
   ) const;

   [[nodiscard]] roaring::Roaring* getFlippedBitmapFromAmbiguousSymbol(
      size_t position,
      GENOME_SYMBOL ambiguous_symbol
   ) const;

   void interpret(const std::vector<std::string>& genomes);

   void indexAllNucleotideSymbolsN();

   void naiveIndexAllNucleotideSymbolN();

   SequenceStoreInfo getInfo() const;
};

[[maybe_unused]] unsigned runOptimize(SequenceStore& sequence_store);

[[maybe_unused]] unsigned shrinkToFit(SequenceStore& sequence_store);

}  // namespace silo

template <>
struct [[maybe_unused]] fmt::formatter<silo::SequenceStoreInfo> : fmt::formatter<std::string> {
   [[maybe_unused]] static auto format(
      silo::SequenceStoreInfo sequence_store_info,
      format_context& ctx
   ) -> decltype(ctx.out());
};

#endif  // SILO_SEQUENCE_STORE_H
