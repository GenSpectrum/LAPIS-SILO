#include "silo/storage/reference_genomes.h"

#include <gtest/gtest.h>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"

TEST(ReferenceGenome, readFromFile) {
   auto under_test =
      silo::ReferenceGenomes::readFromFile("testBaseData/exampleDataset/reference_genomes.json");

   ASSERT_EQ(under_test.nucleotide_sequences.size(), 2UL);
   ASSERT_EQ(under_test.aa_sequences.size(), 12UL);

   ASSERT_EQ(under_test.nucleotide_sequences.at("main").size(), 29903UL);
   ASSERT_EQ(under_test.nucleotide_sequences.at("main").at(0), silo::Nucleotide::Symbol::A);

   ASSERT_EQ(under_test.nucleotide_sequences.at("testSecondSequence").size(), 4UL);
   ASSERT_EQ(
      under_test.nucleotide_sequences.at("testSecondSequence").at(1), silo::Nucleotide::Symbol::C
   );

   ASSERT_EQ(under_test.aa_sequences.at("S").size(), 1274UL);
   ASSERT_EQ(under_test.aa_sequences.at("S").at(3), silo::AminoAcid::Symbol::F);

   ASSERT_EQ(under_test.aa_sequences.at("ORF1a").size(), 4401UL);
   ASSERT_EQ(under_test.aa_sequences.at("ORF1a").at(10), silo::AminoAcid::Symbol::K);

   ASSERT_EQ(under_test.aa_sequences.at("ORF9b").size(), 98UL);
   ASSERT_EQ(under_test.aa_sequences.at("ORF9b").at(10), silo::AminoAcid::Symbol::A);
}
