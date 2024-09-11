#include "silo/storage/position.h"

#include <spdlog/spdlog.h>

template <typename SymbolType>
silo::Position<SymbolType> silo::Position<SymbolType>::fromInitiallyDeleted(
   typename SymbolType::Symbol symbol
) {
   silo::Position<SymbolType> position;
   position.symbol_whose_bitmap_is_deleted = symbol;
   return position;
}

template <typename SymbolType>
silo::Position<SymbolType> silo::Position<SymbolType>::fromInitiallyFlipped(
   typename SymbolType::Symbol symbol
) {
   silo::Position<SymbolType> position;
   //position.symbol_whose_bitmap_is_flipped = symbol;
   return position;
}

template <typename SymbolType>
void silo::Position<SymbolType>::addValues(
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
std::optional<std::pair<typename SymbolType::Symbol, uint32_t>> silo::Position<SymbolType>::getHighestCardinalitySymbol(
   uint32_t sequence_count
) {
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

   if (max_count == 0) {
      return std::nullopt;
   }

   return std::make_pair(max_symbol.value(), max_count);
}

template <typename SymbolType>
std::optional<std::pair<typename SymbolType::Symbol, uint32_t>> silo::Position<SymbolType>::getHighestInformationSymbol(
   uint32_t sequence_count
) {
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
      const uint32_t count = bitmap.getSizeInBytes(false);
      if (count > max_count) {
         max_symbol = symbol;
         max_count = count;
      }
   }

   if (max_count == 0) {
      return std::nullopt;
   }

   return std::make_pair(max_symbol.value(), max_count);
}

template <typename SymbolType>
std::optional<typename SymbolType::Symbol> silo::Position<SymbolType>::flipMostNumerousBitmap(
   uint32_t sequence_count
) {
   if (symbol_whose_bitmap_is_deleted.has_value()) {
      throw std::runtime_error(fmt::format(
         "Symbol '{}' is currently deleted. Cannot restore it implicitly",
         SymbolType::symbolToChar(*symbol_whose_bitmap_is_deleted)
      ));
   }

   auto max_symbol_result = getHighestCardinalitySymbol(sequence_count);

   if (!max_symbol_result.has_value()) {
      return std::nullopt;
   }

   auto [symbol, cardinality] = max_symbol_result.value();

   if (static_cast<double>(cardinality) / static_cast<double>(sequence_count) < 0.5) {
      return std::nullopt;
   }

   if (symbol != symbol_whose_bitmap_is_flipped) {
      if (symbol_whose_bitmap_is_flipped.has_value()) {
         bitmaps[*symbol_whose_bitmap_is_flipped].flip(0, sequence_count);
         bitmaps[*symbol_whose_bitmap_is_flipped].runOptimize();
         bitmaps[*symbol_whose_bitmap_is_flipped].shrinkToFit();
      }
      bitmaps[symbol].flip(0, sequence_count);
      bitmaps[symbol].runOptimize();
      bitmaps[symbol].shrinkToFit();
      symbol_whose_bitmap_is_flipped = symbol;
      return symbol_whose_bitmap_is_flipped;
   }

   return std::nullopt;
}

template <typename SymbolType>
std::optional<typename SymbolType::Symbol> silo::Position<SymbolType>::deleteMostNumerousBitmap(
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

   auto max_symbol_result = getHighestCardinalitySymbol(sequence_count);

   if (!max_symbol_result.has_value()) {
      return std::nullopt;
   }

   auto [symbol, cardinality] = max_symbol_result.value();

//   if (static_cast<double>(cardinality) / static_cast<double>(sequence_count) < 0.5) {
//
//      return std::nullopt;
//   }

   bitmaps[symbol] = roaring::Roaring();
   symbol_whose_bitmap_is_deleted = symbol;
   return symbol_whose_bitmap_is_deleted;
}

template <typename SymbolType>
size_t silo::Position<SymbolType>::computeSize() const {
   size_t result = 0;
   for (const auto symbol : SymbolType::SYMBOLS) {
      result += bitmaps.at(symbol).getSizeInBytes(false);
   }
   return result;
}

template <typename SymbolType>
const roaring::Roaring* silo::Position<SymbolType>::getBitmap(typename SymbolType::Symbol symbol
) const {
   return &bitmaps.at(symbol);
}

template <typename SymbolType>
bool silo::Position<SymbolType>::isSymbolFlipped(typename SymbolType::Symbol symbol) const {
   return symbol == symbol_whose_bitmap_is_flipped;
}

template <typename SymbolType>
bool silo::Position<SymbolType>::isSymbolDeleted(typename SymbolType::Symbol symbol) const {
   return symbol == symbol_whose_bitmap_is_deleted;
}

template <typename SymbolType>
std::optional<typename SymbolType::Symbol> silo::Position<SymbolType>::getDeletedSymbol() const {
   return symbol_whose_bitmap_is_deleted;
}

template class silo::Position<silo::Nucleotide>;
template class silo::Position<silo::AminoAcid>;