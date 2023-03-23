#include "silo/common/genome_symbols.h"

#include <gtest/gtest.h>

TEST(genome_symbol, enum_should_have_same_length_as_symbol_representation) {
   EXPECT_EQ(
      static_cast<unsigned>(silo::GENOME_SYMBOL::LAST) + 1, silo::SYMBOL_REPRESENTATION.size()
   );
}

TEST(genome_symbol, enum_should_have_same_length_as_array_of_symbols) {
   EXPECT_EQ(static_cast<unsigned>(silo::GENOME_SYMBOL::LAST) + 1, silo::GENOME_SYMBOLS.size());
}

TEST(genome_symbol, genome_symbol_representation_works_as_intended) {
   EXPECT_EQ(silo::genomeSymbolRepresentation(silo::GENOME_SYMBOL::GAP), "-");
   EXPECT_EQ(silo::genomeSymbolRepresentation(silo::GENOME_SYMBOL::A), "A");
   EXPECT_EQ(silo::genomeSymbolRepresentation(silo::GENOME_SYMBOL::N), "N");
   EXPECT_THROW(
      silo::genomeSymbolRepresentation(silo::GENOME_SYMBOL::UNDEFINED), std::out_of_range
   );
}

TEST(genome_symbol, to_nucleotide_symbol_works_as_intended) {
   EXPECT_EQ(silo::toNucleotideSymbol('.'), silo::GENOME_SYMBOL::GAP);
   EXPECT_EQ(silo::toNucleotideSymbol('-'), silo::GENOME_SYMBOL::GAP);
   EXPECT_EQ(silo::toNucleotideSymbol('A'), silo::GENOME_SYMBOL::A);
   EXPECT_EQ(silo::toNucleotideSymbol('N'), silo::GENOME_SYMBOL::N);
   EXPECT_EQ(silo::toNucleotideSymbol('X'), silo::GENOME_SYMBOL::GAP);
}
