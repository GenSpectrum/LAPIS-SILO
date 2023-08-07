#include "silo/storage/reference_genomes.h"

#include <gtest/gtest.h>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"

TEST(ReferenceGenome, readFromFile) {
   auto under_test =
      silo::ReferenceGenomes::readFromFile("testBaseData/exampleDataset/reference-genomes.json");

   ASSERT_EQ(under_test.nucleotide_sequences.size(), 2);
   ASSERT_EQ(under_test.aa_sequences.size(), 12);

   ASSERT_EQ(under_test.nucleotide_sequences.at("main").size(), 29903);
   ASSERT_EQ(under_test.nucleotide_sequences.at("main").at(0), silo::NUCLEOTIDE_SYMBOL::A);

   ASSERT_EQ(under_test.nucleotide_sequences.at("testSecondSequence").size(), 4);
   ASSERT_EQ(
      under_test.nucleotide_sequences.at("testSecondSequence").at(1), silo::NUCLEOTIDE_SYMBOL::C
   );

   ASSERT_EQ(under_test.aa_sequences.at("S").size(), 1274);
   ASSERT_EQ(under_test.aa_sequences.at("S").at(3), silo::AA_SYMBOL::F);

   ASSERT_EQ(under_test.aa_sequences.at("ORF1a").size(), 4401);
   ASSERT_EQ(under_test.aa_sequences.at("ORF1a").at(10), silo::AA_SYMBOL::K);

   ASSERT_EQ(under_test.aa_sequences.at("ORF9b").size(), 98);
   ASSERT_EQ(under_test.aa_sequences.at("ORF9b").at(10), silo::AA_SYMBOL::A);
}
