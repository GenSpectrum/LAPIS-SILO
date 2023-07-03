#include "silo/storage/reference_genomes.h"

#include <gtest/gtest.h>

TEST(ReferenceGenome, readFromFile) {
   auto under_test = silo::ReferenceGenomes::readFromFile("testBaseData/reference-genomes.json");

   ASSERT_EQ(under_test.nucleotide_sequences.at("main").size(), 29903);
   ASSERT_EQ(under_test.nucleotide_sequences.at("main").at(0), 'A');
}
