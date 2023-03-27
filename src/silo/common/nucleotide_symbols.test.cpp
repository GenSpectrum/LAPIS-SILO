#include "silo/common/nucleotide_symbols.h"

#include <gtest/gtest.h>

TEST(genome_symbol, enum_should_have_same_length_as_symbol_representation) {
   EXPECT_EQ(silo::SYMBOL_COUNT, silo::SYMBOL_REPRESENTATION.size());
}

TEST(genome_symbol, enum_should_have_same_length_as_array_of_symbols) {
   EXPECT_EQ(silo::SYMBOL_COUNT, silo::GENOME_SYMBOLS.size());
}

TEST(genome_symbol, genome_symbol_representation_works_as_intended) {
   EXPECT_EQ(silo::genomeSymbolRepresentation(silo::NUCLEOTIDE_SYMBOL::GAP), "-");
   EXPECT_EQ(silo::genomeSymbolRepresentation(silo::NUCLEOTIDE_SYMBOL::A), "A");
   EXPECT_EQ(silo::genomeSymbolRepresentation(silo::NUCLEOTIDE_SYMBOL::N), "N");
}

TEST(genome_symbol, to_nucleotide_symbol_works_as_intended) {
   EXPECT_EQ(silo::toNucleotideSymbol('.'), silo::NUCLEOTIDE_SYMBOL::GAP);
   EXPECT_EQ(silo::toNucleotideSymbol('-'), silo::NUCLEOTIDE_SYMBOL::GAP);
   EXPECT_EQ(silo::toNucleotideSymbol('A'), silo::NUCLEOTIDE_SYMBOL::A);
   EXPECT_EQ(silo::toNucleotideSymbol('N'), silo::NUCLEOTIDE_SYMBOL::N);
   EXPECT_EQ(silo::toNucleotideSymbol('X'), silo::NUCLEOTIDE_SYMBOL::GAP);
}
