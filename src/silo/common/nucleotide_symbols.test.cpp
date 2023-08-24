#include "silo/common/nucleotide_symbols.h"

#include <gtest/gtest.h>

TEST(NUCLEOTIDE_SYMBOL, enumShouldHaveSameLengthAsArrayOfSymbols) {
   EXPECT_EQ(
      silo::Util<silo::NUCLEOTIDE_SYMBOL>::count,
      silo::Util<silo::NUCLEOTIDE_SYMBOL>::symbols.size()
   );
}

TEST(NUCLEOTIDE_SYMBOL, conversionFromCharacter) {
   EXPECT_EQ(silo::Util<silo::NUCLEOTIDE_SYMBOL>::charToSymbol('.'), silo::NUCLEOTIDE_SYMBOL::GAP);
   EXPECT_EQ(silo::Util<silo::NUCLEOTIDE_SYMBOL>::charToSymbol('-'), silo::NUCLEOTIDE_SYMBOL::GAP);
   EXPECT_EQ(silo::Util<silo::NUCLEOTIDE_SYMBOL>::charToSymbol('A'), silo::NUCLEOTIDE_SYMBOL::A);
   EXPECT_EQ(silo::Util<silo::NUCLEOTIDE_SYMBOL>::charToSymbol('N'), silo::NUCLEOTIDE_SYMBOL::N);
   EXPECT_EQ(silo::Util<silo::NUCLEOTIDE_SYMBOL>::charToSymbol('X'), std::nullopt);
}

TEST(NUCLEOTIDE_SYMBOL, findIllegalNucleotideChar) {
   EXPECT_EQ(silo::Util<silo::NUCLEOTIDE_SYMBOL>::findIllegalChar("ACGT"), std::nullopt);
   EXPECT_EQ(silo::Util<silo::NUCLEOTIDE_SYMBOL>::findIllegalChar("ACGTZ"), 'Z');
}