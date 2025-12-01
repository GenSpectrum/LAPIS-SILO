#include "silo/common/nucleotide_symbols.h"

#include <gtest/gtest.h>

#include <algorithm>

using silo::Nucleotide;

TEST(NucleotideSymbol, enumShouldHaveSameLengthAsArrayOfSymbols) {
   EXPECT_EQ(Nucleotide::COUNT, Nucleotide::SYMBOLS.size());
}

TEST(NucleotideSymbol, conversionFromCharacter) {
   EXPECT_EQ(Nucleotide::charToSymbol('.'), std::nullopt);
   EXPECT_EQ(Nucleotide::charToSymbol('-'), Nucleotide::Symbol::GAP);
   EXPECT_EQ(Nucleotide::charToSymbol('A'), Nucleotide::Symbol::A);
   EXPECT_EQ(Nucleotide::charToSymbol('N'), Nucleotide::Symbol::N);
   EXPECT_EQ(Nucleotide::charToSymbol('X'), std::nullopt);
}

TEST(NucleotideSymbol, conversionFromCharacterRoundTrip) {
   for (Nucleotide::Symbol symbol : Nucleotide::SYMBOLS) {
      char symbol_character = Nucleotide::symbolToChar(symbol);
      auto maybe_round_tripped_symbol = Nucleotide::charToSymbol(symbol_character);
      ASSERT_TRUE(maybe_round_tripped_symbol.has_value());
      auto round_tripped_symbol = maybe_round_tripped_symbol.value();
      ASSERT_EQ(round_tripped_symbol, symbol);
   }
}

TEST(NucleotideSymbol, ambiguousSymbols) {
   for (const auto& symbol : Nucleotide::SYMBOLS) {
      auto ambiguous_symbols_for_symbol = Nucleotide::AMBIGUITY_SYMBOLS.at(symbol);
      bool contains_self = std::ranges::find(ambiguous_symbols_for_symbol, symbol) !=
                           ambiguous_symbols_for_symbol.end();
      EXPECT_TRUE(contains_self);
   }
}
