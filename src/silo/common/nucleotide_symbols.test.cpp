#include "silo/common/nucleotide_symbols.h"

#include <gtest/gtest.h>

#include <algorithm>

using silo::Nucleotide;
using Symbol = Nucleotide::Symbol;

TEST(NucleotideSymbol, enumShouldHaveSameLengthAsArrayOfSymbols) {
   EXPECT_EQ(Nucleotide::COUNT, Nucleotide::SYMBOLS.size());
}

TEST(NucleotideSymbol, conversionFromCharacter) {
   EXPECT_EQ(Nucleotide::charToSymbol('.'), std::nullopt);
   EXPECT_EQ(Nucleotide::charToSymbol('-'), Symbol::GAP);
   EXPECT_EQ(Nucleotide::charToSymbol('A'), Symbol::A);
   EXPECT_EQ(Nucleotide::charToSymbol('N'), Symbol::N);
   EXPECT_EQ(Nucleotide::charToSymbol('X'), std::nullopt);
}

TEST(NucleotideSymbol, conversionFromCharacterRoundTrip) {
   for (const Symbol symbol : Nucleotide::SYMBOLS) {
      const char symbol_character = Nucleotide::symbolToChar(symbol);
      auto maybe_round_tripped_symbol = Nucleotide::charToSymbol(symbol_character);
      ASSERT_TRUE(maybe_round_tripped_symbol.has_value());
      auto round_tripped_symbol = maybe_round_tripped_symbol.value();
      ASSERT_EQ(round_tripped_symbol, symbol);
   }
}

TEST(NucleotideSymbol, symbolsInOrder) {
   for (auto symbol : Nucleotide::SYMBOLS) {
      auto symbol_at_position = Nucleotide::SYMBOLS.at(static_cast<uint8_t>(symbol));
      EXPECT_EQ(symbol_at_position, symbol);
   }
}

// --- CODES_FOR tests ---

TEST(NucleotideSymbol, codesForConcreteSymbolsCodeForThemselves) {
   for (auto symbol : Nucleotide::VALID_MUTATION_SYMBOLS) {
      const auto& codes_for = Nucleotide::CODES_FOR.at(symbol);
      EXPECT_EQ(codes_for.size(), 1);
      EXPECT_EQ(codes_for.at(0), symbol);
   }
}

// NOLINTBEGIN(readability-function-cognitive-complexity)

TEST(NucleotideSymbol, codesForSpecificAmbiguityCodes) {
   auto contains = [](const std::vector<Symbol>& vec, Symbol symbol) {
      return std::ranges::find(vec, symbol) != vec.end();
   };

   // R = A or G
   {
      const auto& codes = Nucleotide::CODES_FOR.at(Symbol::R);
      EXPECT_EQ(codes.size(), 2);
      EXPECT_TRUE(contains(codes, Symbol::A));
      EXPECT_TRUE(contains(codes, Symbol::G));
   }
   // Y = C or T
   {
      const auto& codes = Nucleotide::CODES_FOR.at(Symbol::Y);
      EXPECT_EQ(codes.size(), 2);
      EXPECT_TRUE(contains(codes, Symbol::C));
      EXPECT_TRUE(contains(codes, Symbol::T));
   }
   // S = G or C
   {
      const auto& codes = Nucleotide::CODES_FOR.at(Symbol::S);
      EXPECT_EQ(codes.size(), 2);
      EXPECT_TRUE(contains(codes, Symbol::G));
      EXPECT_TRUE(contains(codes, Symbol::C));
   }
   // W = A or T
   {
      const auto& codes = Nucleotide::CODES_FOR.at(Symbol::W);
      EXPECT_EQ(codes.size(), 2);
      EXPECT_TRUE(contains(codes, Symbol::A));
      EXPECT_TRUE(contains(codes, Symbol::T));
   }
   // K = G or T
   {
      const auto& codes = Nucleotide::CODES_FOR.at(Symbol::K);
      EXPECT_EQ(codes.size(), 2);
      EXPECT_TRUE(contains(codes, Symbol::G));
      EXPECT_TRUE(contains(codes, Symbol::T));
   }
   // M = A or C
   {
      const auto& codes = Nucleotide::CODES_FOR.at(Symbol::M);
      EXPECT_EQ(codes.size(), 2);
      EXPECT_TRUE(contains(codes, Symbol::A));
      EXPECT_TRUE(contains(codes, Symbol::C));
   }
   // B = C, G, T (not A)
   {
      const auto& codes = Nucleotide::CODES_FOR.at(Symbol::B);
      EXPECT_EQ(codes.size(), 3);
      EXPECT_TRUE(contains(codes, Symbol::C));
      EXPECT_TRUE(contains(codes, Symbol::G));
      EXPECT_TRUE(contains(codes, Symbol::T));
      EXPECT_FALSE(contains(codes, Symbol::A));
   }
   // D = A, G, T (not C)
   {
      const auto& codes = Nucleotide::CODES_FOR.at(Symbol::D);
      EXPECT_EQ(codes.size(), 3);
      EXPECT_TRUE(contains(codes, Symbol::A));
      EXPECT_TRUE(contains(codes, Symbol::G));
      EXPECT_TRUE(contains(codes, Symbol::T));
      EXPECT_FALSE(contains(codes, Symbol::C));
   }
   // H = A, C, T (not G)
   {
      const auto& codes = Nucleotide::CODES_FOR.at(Symbol::H);
      EXPECT_EQ(codes.size(), 3);
      EXPECT_TRUE(contains(codes, Symbol::A));
      EXPECT_TRUE(contains(codes, Symbol::C));
      EXPECT_TRUE(contains(codes, Symbol::T));
      EXPECT_FALSE(contains(codes, Symbol::G));
   }
   // V = A, C, G (not T)
   {
      const auto& codes = Nucleotide::CODES_FOR.at(Symbol::V);
      EXPECT_EQ(codes.size(), 3);
      EXPECT_TRUE(contains(codes, Symbol::A));
      EXPECT_TRUE(contains(codes, Symbol::C));
      EXPECT_TRUE(contains(codes, Symbol::G));
      EXPECT_FALSE(contains(codes, Symbol::T));
   }
   // N = all symbols
   {
      const auto& codes = Nucleotide::CODES_FOR.at(Symbol::N);
      EXPECT_EQ(codes.size(), Nucleotide::COUNT);
   }
}

// --- AMBIGUITY_SYMBOLS tests ---

TEST(NucleotideSymbol, ambiguousSymbolsContainSelf) {
   for (auto symbol : Nucleotide::SYMBOLS) {
      const auto& ambiguous = Nucleotide::AMBIGUITY_SYMBOLS.at(symbol);
      const bool contains_self = std::ranges::find(ambiguous, symbol) != ambiguous.end();
      EXPECT_TRUE(contains_self) << "Symbol " << Nucleotide::symbolToChar(symbol)
                                 << " should be in its own AMBIGUITY_SYMBOLS";
   }
}

TEST(NucleotideSymbol, ambiguousSymbolsContainN) {
   for (auto symbol : Nucleotide::SYMBOLS) {
      const auto& ambiguous = Nucleotide::AMBIGUITY_SYMBOLS.at(symbol);
      const bool contains_n = std::ranges::find(ambiguous, Symbol::N) != ambiguous.end();
      EXPECT_TRUE(contains_n) << "Symbol " << Nucleotide::symbolToChar(symbol)
                              << " should have N in its AMBIGUITY_SYMBOLS";
   }
}

TEST(NucleotideSymbol, ambiguitySymbolsAreSupersetRelationOfCodesFor) {
   // Verify AMBIGUITY_SYMBOLS[S] = {Y : CODES_FOR[S] ⊆ CODES_FOR[Y]}
   for (auto symbol : Nucleotide::SYMBOLS) {
      const auto& codes_for_symbol = Nucleotide::CODES_FOR.at(symbol);
      const auto& ambiguous = Nucleotide::AMBIGUITY_SYMBOLS.at(symbol);

      for (auto candidate : Nucleotide::SYMBOLS) {
         const auto& codes_for_candidate = Nucleotide::CODES_FOR.at(candidate);

         const bool is_superset = std::ranges::all_of(codes_for_symbol, [&](auto coded_symbol) {
            return std::ranges::find(codes_for_candidate, coded_symbol) !=
                   codes_for_candidate.end();
         });
         const bool in_ambiguous = std::ranges::find(ambiguous, candidate) != ambiguous.end();

         EXPECT_EQ(is_superset, in_ambiguous)
            << "Symbol " << Nucleotide::symbolToChar(symbol) << " vs candidate "
            << Nucleotide::symbolToChar(candidate) << ": superset=" << is_superset
            << " in_ambiguous=" << in_ambiguous;
      }
   }
}

TEST(NucleotideSymbol, ambiguitySymbolsSpecificExamples) {
   auto contains = [](const std::vector<Symbol>& vec, Symbol symbol) {
      return std::ranges::find(vec, symbol) != vec.end();
   };

   // A should be matched by R(AG), M(AC), W(AT), D(AGT), H(ACT), V(ACG), N
   {
      const auto& amb = Nucleotide::AMBIGUITY_SYMBOLS.at(Symbol::A);
      EXPECT_TRUE(contains(amb, Symbol::A));
      EXPECT_TRUE(contains(amb, Symbol::R));
      EXPECT_TRUE(contains(amb, Symbol::M));
      EXPECT_TRUE(contains(amb, Symbol::W));
      EXPECT_TRUE(contains(amb, Symbol::D));
      EXPECT_TRUE(contains(amb, Symbol::H));
      EXPECT_TRUE(contains(amb, Symbol::V));
      EXPECT_TRUE(contains(amb, Symbol::N));
      EXPECT_FALSE(contains(amb, Symbol::Y));
      EXPECT_FALSE(contains(amb, Symbol::S));
      EXPECT_FALSE(contains(amb, Symbol::K));
      EXPECT_FALSE(contains(amb, Symbol::B));
   }

   // GAP should be matched by GAP and N
   {
      const auto& amb = Nucleotide::AMBIGUITY_SYMBOLS.at(Symbol::GAP);
      EXPECT_TRUE(contains(amb, Symbol::GAP));
      EXPECT_TRUE(contains(amb, Symbol::N));
      EXPECT_EQ(amb.size(), 2);
   }

   // R(AG) should be matched by R, D(AGT), V(ACG), N
   {
      const auto& amb = Nucleotide::AMBIGUITY_SYMBOLS.at(Symbol::R);
      EXPECT_TRUE(contains(amb, Symbol::R));
      EXPECT_TRUE(contains(amb, Symbol::D));
      EXPECT_TRUE(contains(amb, Symbol::V));
      EXPECT_TRUE(contains(amb, Symbol::N));
      EXPECT_EQ(amb.size(), 4);
   }

   // B(CGT) should be matched by B and N only
   {
      const auto& amb = Nucleotide::AMBIGUITY_SYMBOLS.at(Symbol::B);
      EXPECT_TRUE(contains(amb, Symbol::B));
      EXPECT_TRUE(contains(amb, Symbol::N));
      EXPECT_EQ(amb.size(), 2);
   }

   // N should only be matched by N itself
   {
      const auto& amb = Nucleotide::AMBIGUITY_SYMBOLS.at(Symbol::N);
      EXPECT_EQ(amb.size(), 1);
      EXPECT_TRUE(contains(amb, Symbol::N));
   }
}

// NOLINTEND(readability-function-cognitive-complexity)
