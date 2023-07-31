#include "silo/common/nucleotide_symbols.h"

#include <gtest/gtest.h>

TEST(NucleotideSymbol, enumShouldHaveSameLengthAsArrayOfSymbols) {
   EXPECT_EQ(silo::NUC_SYMBOL_COUNT, silo::NUC_SYMBOLS.size());
}

TEST(NucleotideSymbol, conversionFromCharacter) {
   EXPECT_EQ(silo::charToNucleotideSymbol('.'), silo::NUCLEOTIDE_SYMBOL::GAP);
   EXPECT_EQ(silo::charToNucleotideSymbol('-'), silo::NUCLEOTIDE_SYMBOL::GAP);
   EXPECT_EQ(silo::charToNucleotideSymbol('A'), silo::NUCLEOTIDE_SYMBOL::A);
   EXPECT_EQ(silo::charToNucleotideSymbol('N'), silo::NUCLEOTIDE_SYMBOL::N);
   EXPECT_EQ(silo::charToNucleotideSymbol('X'), std::nullopt);
}

TEST(NucleotideSymbol, findIllegalNucleotideChar) {
   EXPECT_EQ(silo::findIllegalNucleotideChar("ACGT"), std::nullopt);
   EXPECT_EQ(silo::findIllegalNucleotideChar("ACGTZ"), 'Z');
}