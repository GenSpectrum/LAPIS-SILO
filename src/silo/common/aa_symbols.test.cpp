#include "silo/common/aa_symbols.h"

#include <algorithm>
#include <array>
#include <optional>

#include <gtest/gtest.h>

using silo::AminoAcid;
using Symbol = AminoAcid::Symbol;

TEST(AminoAcidSymbol, enumShouldHaveSameLengthAsArrayOfSymbols) {
   EXPECT_EQ(AminoAcid::COUNT, AminoAcid::SYMBOLS.size());
}

TEST(AminoAcidSymbol, conversionFromCharacter) {
   EXPECT_EQ(AminoAcid::charToSymbol('-'), Symbol::GAP);
   EXPECT_EQ(AminoAcid::charToSymbol('.'), std::nullopt);
   EXPECT_EQ(AminoAcid::charToSymbol('A'), Symbol::A);
   EXPECT_EQ(AminoAcid::charToSymbol('N'), Symbol::N);
   EXPECT_EQ(AminoAcid::charToSymbol('J'), Symbol::J);
   EXPECT_EQ(AminoAcid::charToSymbol(')'), std::nullopt);
}

TEST(AminoAcidSymbol, conversionFromCharacterRoundTrip) {
   for (const Symbol symbol : AminoAcid::SYMBOLS) {
      const char symbol_character = AminoAcid::symbolToChar(symbol);
      auto maybe_round_tripped_symbol = AminoAcid::charToSymbol(symbol_character);
      ASSERT_TRUE(maybe_round_tripped_symbol.has_value());
      auto round_tripped_symbol = maybe_round_tripped_symbol.value();
      ASSERT_EQ(round_tripped_symbol, symbol);
   }
}

TEST(AminoAcidSymbol, symbolsInOrder) {
   for (auto symbol : AminoAcid::SYMBOLS) {
      auto symbol_at_position = AminoAcid::SYMBOLS.at(static_cast<uint8_t>(symbol));
      EXPECT_EQ(symbol_at_position, symbol);
   }
}

// NOLINTBEGIN(readability-function-cognitive-complexity)

// --- CODES_FOR tests ---

TEST(AminoAcidSymbol, codesForConcreteSymbolsCodeForThemselves) {
   for (auto symbol : AminoAcid::VALID_MUTATION_SYMBOLS) {
      const auto& codes_for = AminoAcid::CODES_FOR.at(symbol);
      EXPECT_EQ(codes_for.size(), 1) << "Concrete symbol " << AminoAcid::symbolToChar(symbol)
                                     << " should code for exactly itself";
      EXPECT_EQ(codes_for.at(0), symbol);
   }
}

TEST(AminoAcidSymbol, codesForSpecificAmbiguityCodes) {
   auto contains = [](const std::vector<Symbol>& vec, Symbol symbol) {
      return std::ranges::find(vec, symbol) != vec.end();
   };

   // B = D (Aspartic acid) or N (Asparagine)
   {
      const auto& codes = AminoAcid::CODES_FOR.at(Symbol::B);
      EXPECT_EQ(codes.size(), 2);
      EXPECT_TRUE(contains(codes, Symbol::D));
      EXPECT_TRUE(contains(codes, Symbol::N));
   }
   // J = L (Leucine) or I (Isoleucine)
   {
      const auto& codes = AminoAcid::CODES_FOR.at(Symbol::J);
      EXPECT_EQ(codes.size(), 2);
      EXPECT_TRUE(contains(codes, Symbol::L));
      EXPECT_TRUE(contains(codes, Symbol::I));
   }
   // Z = Q (Glutamine) or E (Glutamic acid)
   {
      const auto& codes = AminoAcid::CODES_FOR.at(Symbol::Z);
      EXPECT_EQ(codes.size(), 2);
      EXPECT_TRUE(contains(codes, Symbol::Q));
      EXPECT_TRUE(contains(codes, Symbol::E));
   }
   // X = all symbols
   {
      const auto& codes = AminoAcid::CODES_FOR.at(Symbol::X);
      EXPECT_EQ(codes.size(), AminoAcid::COUNT);
   }
}

// --- AMBIGUITY_SYMBOLS tests ---

TEST(AminoAcidSymbol, ambiguousSymbolsContainSelf) {
   for (auto symbol : AminoAcid::SYMBOLS) {
      const auto& ambiguous = AminoAcid::AMBIGUITY_SYMBOLS.at(symbol);
      const bool contains_self = std::ranges::find(ambiguous, symbol) != ambiguous.end();
      EXPECT_TRUE(contains_self) << "Symbol " << AminoAcid::symbolToChar(symbol)
                                 << " should be in its own AMBIGUITY_SYMBOLS";
   }
}

TEST(AminoAcidSymbol, ambiguousSymbolsContainX) {
   for (auto symbol : AminoAcid::SYMBOLS) {
      const auto& ambiguous = AminoAcid::AMBIGUITY_SYMBOLS.at(symbol);
      const bool contains_x = std::ranges::find(ambiguous, Symbol::X) != ambiguous.end();
      EXPECT_TRUE(contains_x) << "Symbol " << AminoAcid::symbolToChar(symbol)
                              << " should have X in its AMBIGUITY_SYMBOLS";
   }
}

TEST(AminoAcidSymbol, ambiguitySymbolsAreSupersetRelationOfCodesFor) {
   // Verify AMBIGUITY_SYMBOLS[S] = {Y : CODES_FOR[S] ⊆ CODES_FOR[Y]}
   for (auto symbol : AminoAcid::SYMBOLS) {
      const auto& codes_for_symbol = AminoAcid::CODES_FOR.at(symbol);
      const auto& ambiguous = AminoAcid::AMBIGUITY_SYMBOLS.at(symbol);

      for (auto candidate : AminoAcid::SYMBOLS) {
         const auto& codes_for_candidate = AminoAcid::CODES_FOR.at(candidate);

         const bool is_superset = std::ranges::all_of(codes_for_symbol, [&](auto coded_symbol) {
            return std::ranges::find(codes_for_candidate, coded_symbol) !=
                   codes_for_candidate.end();
         });
         const bool in_ambiguous = std::ranges::find(ambiguous, candidate) != ambiguous.end();

         EXPECT_EQ(is_superset, in_ambiguous)
            << "Symbol " << AminoAcid::symbolToChar(symbol) << " vs candidate "
            << AminoAcid::symbolToChar(candidate) << ": superset=" << is_superset
            << " in_ambiguous=" << in_ambiguous;
      }
   }
}

TEST(AminoAcidSymbol, ambiguitySymbolsSpecificExamples) {
   auto contains = [](const std::vector<Symbol>& vec, Symbol symbol) {
      return std::ranges::find(vec, symbol) != vec.end();
   };

   // L (Leucine) should be matched by L, J (Leucine or Isoleucine), X
   {
      const auto& amb = AminoAcid::AMBIGUITY_SYMBOLS.at(Symbol::L);
      EXPECT_TRUE(contains(amb, Symbol::L));
      EXPECT_TRUE(contains(amb, Symbol::J));
      EXPECT_TRUE(contains(amb, Symbol::X));
      EXPECT_EQ(amb.size(), 3);
   }

   // K (Lysine) should NOT be matched by J (J = Leucine or Isoleucine)
   {
      const auto& amb = AminoAcid::AMBIGUITY_SYMBOLS.at(Symbol::K);
      EXPECT_TRUE(contains(amb, Symbol::K));
      EXPECT_TRUE(contains(amb, Symbol::X));
      EXPECT_FALSE(contains(amb, Symbol::J));
      EXPECT_EQ(amb.size(), 2);
   }

   // D (Aspartic Acid) should be matched by D, B (D or N), X
   {
      const auto& amb = AminoAcid::AMBIGUITY_SYMBOLS.at(Symbol::D);
      EXPECT_TRUE(contains(amb, Symbol::D));
      EXPECT_TRUE(contains(amb, Symbol::B));
      EXPECT_TRUE(contains(amb, Symbol::X));
      EXPECT_EQ(amb.size(), 3);
   }

   // N (Asparagine) should be matched by N, B (D or N), X
   {
      const auto& amb = AminoAcid::AMBIGUITY_SYMBOLS.at(Symbol::N);
      EXPECT_TRUE(contains(amb, Symbol::N));
      EXPECT_TRUE(contains(amb, Symbol::B));
      EXPECT_TRUE(contains(amb, Symbol::X));
      EXPECT_EQ(amb.size(), 3);
   }

   // E (Glutamic Acid) should be matched by E, Z (Q or E), X
   {
      const auto& amb = AminoAcid::AMBIGUITY_SYMBOLS.at(Symbol::E);
      EXPECT_TRUE(contains(amb, Symbol::E));
      EXPECT_TRUE(contains(amb, Symbol::Z));
      EXPECT_TRUE(contains(amb, Symbol::X));
      EXPECT_EQ(amb.size(), 3);
   }

   // GAP should be matched by GAP and X
   {
      const auto& amb = AminoAcid::AMBIGUITY_SYMBOLS.at(Symbol::GAP);
      EXPECT_TRUE(contains(amb, Symbol::GAP));
      EXPECT_TRUE(contains(amb, Symbol::X));
      EXPECT_EQ(amb.size(), 2);
   }

   // X should only be matched by X itself
   {
      const auto& amb = AminoAcid::AMBIGUITY_SYMBOLS.at(Symbol::X);
      EXPECT_EQ(amb.size(), 1);
      EXPECT_TRUE(contains(amb, Symbol::X));
   }

   // B should be matched by B and X only
   {
      const auto& amb = AminoAcid::AMBIGUITY_SYMBOLS.at(Symbol::B);
      EXPECT_TRUE(contains(amb, Symbol::B));
      EXPECT_TRUE(contains(amb, Symbol::X));
      EXPECT_EQ(amb.size(), 2);
   }
}

// NOLINTEND(readability-function-cognitive-complexity)
