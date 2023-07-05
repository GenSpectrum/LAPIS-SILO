#include "silo/common/nucleotide_symbols.h"

#include <gtest/gtest.h>

TEST(NucleotideSymbol, enumShouldHaveSameLengthAsSymbolRepresentation) {
   EXPECT_EQ(silo::NUC_SYMBOL_COUNT, silo::NUC_SYMBOL_REPRESENTATION.size());
}

TEST(NucleotideSymbol, enumShouldHaveSameLengthAsArrayOfSymbols) {
   EXPECT_EQ(silo::NUC_SYMBOL_COUNT, silo::NUC_SYMBOLS.size());
}

TEST(NucleotideSymbol, genomeSymbolRepresentationAsString) {
   EXPECT_EQ(silo::genomeSymbolRepresentation(silo::NUCLEOTIDE_SYMBOL::GAP), "-");
   EXPECT_EQ(silo::genomeSymbolRepresentation(silo::NUCLEOTIDE_SYMBOL::A), "A");
   EXPECT_EQ(silo::genomeSymbolRepresentation(silo::NUCLEOTIDE_SYMBOL::N), "N");
}

TEST(NucleotideSymbol, conversionFromCharacter) {
   EXPECT_EQ(silo::toNucleotideSymbol('.'), silo::NUCLEOTIDE_SYMBOL::GAP);
   EXPECT_EQ(silo::toNucleotideSymbol('-'), silo::NUCLEOTIDE_SYMBOL::GAP);
   EXPECT_EQ(silo::toNucleotideSymbol('A'), silo::NUCLEOTIDE_SYMBOL::A);
   EXPECT_EQ(silo::toNucleotideSymbol('N'), silo::NUCLEOTIDE_SYMBOL::N);
   EXPECT_EQ(silo::toNucleotideSymbol('X'), std::nullopt);
}
