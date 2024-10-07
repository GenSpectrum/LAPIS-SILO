#include "silo/storage/reference_genomes.h"

#include <gtest/gtest.h>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"

TEST(ReferenceGenome, readFromFile) {
   auto under_test =
      silo::ReferenceGenomes::readFromFile("testBaseData/exampleDatasetAsTsv/reference_genomes.json"
      );

   ASSERT_EQ(under_test.nucleotide_sequences.size(), 2UL);
   ASSERT_EQ(under_test.aa_sequences.size(), 12UL);

   ASSERT_EQ(under_test.nucleotide_sequence_names.at(0), "main");
   ASSERT_EQ(under_test.nucleotide_sequences.at(0).size(), 29903UL);
   ASSERT_EQ(under_test.nucleotide_sequences.at(0).at(0), silo::Nucleotide::Symbol::A);

   ASSERT_EQ(under_test.nucleotide_sequence_names.at(1), "testSecondSequence");
   ASSERT_EQ(under_test.nucleotide_sequences.at(1).size(), 4UL);
   ASSERT_EQ(under_test.nucleotide_sequences.at(1).at(1), silo::Nucleotide::Symbol::C);

   ASSERT_EQ(under_test.aa_sequence_names.at(11), "S");
   ASSERT_EQ(under_test.aa_sequences.at(11).size(), 1274UL);
   ASSERT_EQ(under_test.aa_sequences.at(11).at(3), silo::AminoAcid::Symbol::F);

   ASSERT_EQ(under_test.aa_sequence_names.at(3), "ORF1a");
   ASSERT_EQ(under_test.aa_sequences.at(3).size(), 4401UL);
   ASSERT_EQ(under_test.aa_sequences.at(3).at(10), silo::AminoAcid::Symbol::K);

   ASSERT_EQ(under_test.aa_sequence_names.at(10), "ORF9b");
   ASSERT_EQ(under_test.aa_sequences.at(10).size(), 98UL);
   ASSERT_EQ(under_test.aa_sequences.at(10).at(10), silo::AminoAcid::Symbol::A);
}
