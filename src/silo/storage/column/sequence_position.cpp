#include "silo/storage/column/sequence_position.h"

#include <spdlog/spdlog.h>

namespace silo::storage::column {

template <typename SymbolType>
SequencePosition<SymbolType> SequencePosition<SymbolType>::fromInitiallyDeleted(
   typename SymbolType::Symbol symbol
) {
   SequencePosition<SymbolType> position;
   position.symbol_whose_bitmap_is_deleted = symbol;
   return position;
}

template <typename SymbolType>
SequencePosition<SymbolType> SequencePosition<SymbolType>::fromInitiallyFlipped(
   typename SymbolType::Symbol symbol
) {
   SequencePosition<SymbolType> position;
   position.symbol_whose_bitmap_is_flipped = symbol;
   return position;
}

template <typename SymbolType>
void SequencePosition<SymbolType>::addValues(
   typename SymbolType::Symbol symbol,
   const std::vector<uint32_t>& values,
   size_t current_offset,
   size_t interval_size
) {
   if (symbol == symbol_whose_bitmap_is_deleted) {
      return;
   }
   if (!values.empty()) {
      bitmaps[symbol].addMany(values.size(), values.data());
   }
   if (symbol == symbol_whose_bitmap_is_flipped) {
      bitmaps[symbol].flip(current_offset, current_offset + interval_size);
   }
}

template <typename SymbolType>
std::optional<typename SymbolType::Symbol> SequencePosition<
   SymbolType>::getHighestCardinalitySymbol(uint32_t sequence_count) {
   if (symbol_whose_bitmap_is_deleted.has_value()) {
      throw std::runtime_error(fmt::format(
         "Symbol '{}' is currently deleted. Cannot restore it implicitly and cannot calculate its "
         "cardinality as we do not have information about missing symbols",
         SymbolType::symbolToChar(*symbol_whose_bitmap_is_deleted)
      ));
   }

   std::optional<typename SymbolType::Symbol> max_symbol = std::nullopt;
   uint32_t max_count = 0;

   for (const auto& symbol : SymbolType::SYMBOLS) {
      roaring::Roaring& bitmap = bitmaps[symbol];
      bitmap.runOptimize();
      bitmap.shrinkToFit();
      const uint32_t count =
         isSymbolFlipped(symbol) ? sequence_count - bitmap.cardinality() : bitmap.cardinality();
      if (count > max_count) {
         max_symbol = symbol;
         max_count = count;
      }
   }
   return max_symbol;
}

template <typename SymbolType>
std::optional<typename SymbolType::Symbol> SequencePosition<SymbolType>::flipMostNumerousBitmap(
   uint32_t sequence_count
) {
   if (symbol_whose_bitmap_is_deleted.has_value()) {
      throw std::runtime_error(fmt::format(
         "Symbol '{}' is currently deleted. Cannot restore it implicitly",
         SymbolType::symbolToChar(*symbol_whose_bitmap_is_deleted)
      ));
   }

   std::optional<typename SymbolType::Symbol> max_symbol =
      getHighestCardinalitySymbol(sequence_count);

   if (max_symbol != symbol_whose_bitmap_is_flipped) {
      if (symbol_whose_bitmap_is_flipped.has_value()) {
         bitmaps[*symbol_whose_bitmap_is_flipped].flip(0, sequence_count);
         bitmaps[*symbol_whose_bitmap_is_flipped].runOptimize();
         bitmaps[*symbol_whose_bitmap_is_flipped].shrinkToFit();
      }
      if (max_symbol.has_value()) {
         bitmaps[*max_symbol].flip(0, sequence_count);
         bitmaps[*max_symbol].runOptimize();
         bitmaps[*max_symbol].shrinkToFit();
      }
      symbol_whose_bitmap_is_flipped = max_symbol;
      return symbol_whose_bitmap_is_flipped;
   }
   return std::nullopt;
}

template <typename SymbolType>
std::optional<typename SymbolType::Symbol> SequencePosition<SymbolType>::deleteMostNumerousBitmap(
   uint32_t sequence_count
) {
   if (symbol_whose_bitmap_is_deleted.has_value()) {
      throw std::runtime_error(fmt::format(
         "Symbol '{}' is currently deleted. Cannot restore it implicitly",
         SymbolType::symbolToChar(*symbol_whose_bitmap_is_deleted)
      ));
   }
   if (symbol_whose_bitmap_is_flipped.has_value()) {
      bitmaps[*symbol_whose_bitmap_is_flipped].flip(0, sequence_count);
      bitmaps[*symbol_whose_bitmap_is_flipped].runOptimize();
      bitmaps[*symbol_whose_bitmap_is_flipped].shrinkToFit();
      symbol_whose_bitmap_is_flipped = std::nullopt;
   }

   std::optional<typename SymbolType::Symbol> max_symbol =
      getHighestCardinalitySymbol(sequence_count);

   if (max_symbol.has_value()) {
      bitmaps[*max_symbol] = roaring::Roaring();
      symbol_whose_bitmap_is_deleted = max_symbol;
      return symbol_whose_bitmap_is_deleted;
   }
   return std::nullopt;
}

template <typename SymbolType>
size_t SequencePosition<SymbolType>::computeSize() const {
   size_t result = 0;
   for (const auto symbol : SymbolType::SYMBOLS) {
      result += bitmaps.at(symbol).getSizeInBytes(false);
   }
   return result;
}

template <typename SymbolType>
const roaring::Roaring* SequencePosition<SymbolType>::getBitmap(typename SymbolType::Symbol symbol
) const {
   return &bitmaps.at(symbol);
}

template <typename SymbolType>
bool SequencePosition<SymbolType>::isSymbolFlipped(typename SymbolType::Symbol symbol) const {
   return symbol == symbol_whose_bitmap_is_flipped;
}

template <typename SymbolType>
bool SequencePosition<SymbolType>::isSymbolDeleted(typename SymbolType::Symbol symbol) const {
   return symbol == symbol_whose_bitmap_is_deleted;
}

template <typename SymbolType>
std::optional<typename SymbolType::Symbol> SequencePosition<SymbolType>::getDeletedSymbol() const {
   return symbol_whose_bitmap_is_deleted;
}

template class SequencePosition<silo::Nucleotide>;
template class SequencePosition<silo::AminoAcid>;

}  // namespace silo::storage::column
