#include "silo/common/nucleotide_symbols.h"

#include <gtest/gtest.h>

TEST(NucleotideSymbol, enumShouldHaveSameLengthAsArrayOfSymbols) {
   EXPECT_EQ(silo::Nucleotide::COUNT, silo::Nucleotide::SYMBOLS.size());
}

TEST(NucleotideSymbol, conversionFromCharacter) {
   EXPECT_EQ(silo::Nucleotide::charToSymbol('.'), std::nullopt);
   EXPECT_EQ(silo::Nucleotide::charToSymbol('-'), silo::Nucleotide::Symbol::GAP);
   EXPECT_EQ(silo::Nucleotide::charToSymbol('A'), silo::Nucleotide::Symbol::A);
   EXPECT_EQ(silo::Nucleotide::charToSymbol('N'), silo::Nucleotide::Symbol::N);
   EXPECT_EQ(silo::Nucleotide::charToSymbol('X'), std::nullopt);
}
