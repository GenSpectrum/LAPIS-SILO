#include "silo/common/aa_symbols.h"

#include <algorithm>
#include <array>
#include <optional>

#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

using silo::AminoAcid;

TEST(AminoAcidSymbol, enumShouldHaveSameLengthAsArrayOfSymbols) {
   EXPECT_EQ(AminoAcid::COUNT, AminoAcid::SYMBOLS.size());
}

TEST(AminoAcidSymbol, conversionFromCharacter) {
   EXPECT_EQ(silo::AminoAcid::charToSymbol('-'), silo::AminoAcid::Symbol::GAP);
   EXPECT_EQ(silo::AminoAcid::charToSymbol('.'), std::nullopt);
   EXPECT_EQ(silo::AminoAcid::charToSymbol('A'), silo::AminoAcid::Symbol::A);
   EXPECT_EQ(silo::AminoAcid::charToSymbol('N'), silo::AminoAcid::Symbol::N);
   EXPECT_EQ(silo::AminoAcid::charToSymbol('J'), silo::AminoAcid::Symbol::J);
   EXPECT_EQ(silo::AminoAcid::charToSymbol(')'), std::nullopt);
}

TEST(AminoAcidSymbol, conversionFromCharacterRoundTrip) {
   for (AminoAcid::Symbol symbol : AminoAcid::SYMBOLS) {
      char symbol_character = AminoAcid::symbolToChar(symbol);
      auto maybe_round_tripped_symbol = AminoAcid::charToSymbol(symbol_character);
      ASSERT_TRUE(maybe_round_tripped_symbol.has_value());
      auto round_tripped_symbol = maybe_round_tripped_symbol.value();
      ASSERT_EQ(round_tripped_symbol, symbol);
   }
}

TEST(AminoAcidSymbol, ambiguousSymbols) {
   for (const auto& symbol : AminoAcid::SYMBOLS) {
      auto ambiguous_symbols_for_symbol = AminoAcid::AMBIGUITY_SYMBOLS.at(symbol);
      bool contains_self = std::ranges::find(ambiguous_symbols_for_symbol, symbol) !=
                           ambiguous_symbols_for_symbol.end();
      bool contains_x = std::ranges::find(ambiguous_symbols_for_symbol, AminoAcid::Symbol::X) !=
                        ambiguous_symbols_for_symbol.end();
      EXPECT_TRUE(contains_self);
      EXPECT_TRUE(contains_x);
   }
}

TEST(AminoAcidSymbol, symbolsInOrder) {
   for (const auto& symbol : AminoAcid::SYMBOLS) {
      auto symbol_at_symbol_position = AminoAcid::SYMBOLS.at(static_cast<uint8_t>(symbol));
      SPDLOG_INFO(
         "{} {}",
         AminoAcid::symbolToChar(symbol_at_symbol_position),
         AminoAcid::symbolToChar(symbol)
      );
      EXPECT_EQ(symbol_at_symbol_position, symbol);
   }
}
