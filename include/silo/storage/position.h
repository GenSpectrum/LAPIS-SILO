#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

#include <fmt/format.h>
#include <roaring/roaring.hh>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/common/symbol_map.h"

namespace boost::serialization {
class access;
}  // namespace boost::serialization

namespace silo {

template <typename SymbolType>
class Position {
   friend class boost::serialization::access;

   template <class Archive>
   void serialize(Archive& archive, [[maybe_unused]] const uint32_t version) {
      // clang-format off
      archive & bitmaps;
      archive & symbol_whose_bitmap_is_flipped;
      archive & symbol_whose_bitmap_is_deleted;
      // clang-format on
   }

   SymbolMap<SymbolType, roaring::Roaring> bitmaps;

  public:
   Position() = default;

      std::optional<typename SymbolType::Symbol> symbol_whose_bitmap_is_flipped;
      std::optional<typename SymbolType::Symbol> symbol_whose_bitmap_is_deleted;

   static Position<SymbolType> fromInitiallyDeleted(typename SymbolType::Symbol symbol);
   static Position<SymbolType> fromInitiallyFlipped(typename SymbolType::Symbol symbol);

   void addValues(
      typename SymbolType::Symbol symbol,
      const std::vector<uint32_t>& values,
      size_t current_offset,
      size_t interval_size
   );

   std::optional<typename SymbolType::Symbol> flipMostNumerousBitmap(uint32_t sequence_count);

   std::optional<typename SymbolType::Symbol> deleteMostNumerousBitmap(uint32_t sequence_count);

   std::optional<std::pair<typename SymbolType::Symbol, uint32_t>> getHighestCardinalitySymbol(uint32_t sequence_count);
   std::optional<std::pair<typename SymbolType::Symbol, uint32_t>> getHighestInformationSymbol(uint32_t sequence_count);

   size_t computeSize() const;

   bool isSymbolFlipped(typename SymbolType::Symbol) const;

   bool isSymbolDeleted(typename SymbolType::Symbol) const;
   std::optional<typename SymbolType::Symbol> getDeletedSymbol() const;

   const roaring::Roaring* getBitmap(typename SymbolType::Symbol symbol) const;
};

}  // namespace silo