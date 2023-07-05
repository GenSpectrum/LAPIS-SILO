#include "silo/storage/reference_genomes.h"

#include <gtest/gtest.h>

TEST(ReferenceGenome, readFromFile) {
   auto under_test = silo::ReferenceGenomes::readFromFile("testBaseData/reference-genomes.json");

   ASSERT_EQ(under_test.nucleotide_sequences.size(), 1);
   ASSERT_EQ(under_test.aa_sequences.size(), 12);

   ASSERT_EQ(under_test.nucleotide_sequences.at("main").size(), 29903);
   ASSERT_EQ(under_test.nucleotide_sequences.at("main").at(0), 'A');

   ASSERT_EQ(under_test.aa_sequences.at("S").size(), 1274);
   ASSERT_EQ(under_test.aa_sequences.at("S").at(3), 'F');

   ASSERT_EQ(under_test.aa_sequences.at("ORF1a").size(), 4401);
   ASSERT_EQ(under_test.aa_sequences.at("ORF1a").at(10), 'K');

   ASSERT_EQ(under_test.aa_sequences.at("ORF9b").size(), 98);
   ASSERT_EQ(under_test.aa_sequences.at("ORF9b").at(10), 'A');
}
