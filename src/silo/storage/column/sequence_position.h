#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

#include <fmt/format.h>
#include <boost/serialization/access.hpp>
#include <roaring/roaring.hh>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/common/symbol_map.h"

namespace silo::storage::column {

template <typename SymbolType>
class SequencePosition {
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
   std::optional<typename SymbolType::Symbol> symbol_whose_bitmap_is_flipped;
   std::optional<typename SymbolType::Symbol> symbol_whose_bitmap_is_deleted;

   std::optional<typename SymbolType::Symbol> getHighestCardinalitySymbol(uint32_t sequence_count);

  public:
   SequencePosition() = default;

   static SequencePosition<SymbolType> fromInitiallyDeleted(typename SymbolType::Symbol symbol);
   static SequencePosition<SymbolType> fromInitiallyFlipped(typename SymbolType::Symbol symbol);

   void addValues(
      typename SymbolType::Symbol symbol,
      const std::vector<uint32_t>& values,
      size_t current_offset,
      size_t interval_size
   );

   std::optional<typename SymbolType::Symbol> flipMostNumerousBitmap(uint32_t sequence_count);

   std::optional<typename SymbolType::Symbol> deleteMostNumerousBitmap(uint32_t sequence_count);

   size_t computeSize() const;

   bool isSymbolFlipped(typename SymbolType::Symbol) const;

   bool isSymbolDeleted(typename SymbolType::Symbol) const;
   std::optional<typename SymbolType::Symbol> getDeletedSymbol() const;

   const roaring::Roaring* getBitmap(typename SymbolType::Symbol symbol) const;
};

}  // namespace silo::storage::column