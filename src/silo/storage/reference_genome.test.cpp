#include "silo/storage/reference_genome.h"

#include <gtest/gtest.h>

TEST(ReferenceGenome, readFromFile) {
   auto under_test = silo::ReferenceGenome::readFromFile("testBaseData/reference_genome.txt");

   ASSERT_EQ(under_test.genome[0].size(), 29903);
   ASSERT_EQ(under_test.genome[0].at(0), 'A');
}