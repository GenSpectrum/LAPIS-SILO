#include "silo/storage/sequence_store.h"

#include <gtest/gtest.h>

#include <memory>
#include <string>

std::unique_ptr<silo::SequenceStore> setupSequenceStore() {
   std::unique_ptr<silo::SequenceStore> sequence_store = std::make_unique<silo::SequenceStore>();

   // Construct test genomes.
   std::string test_genome1 = "A";
   test_genome1.reserve(silo::GENOME_LENGTH);
   for (unsigned i = 1; i < silo::GENOME_LENGTH; ++i) {
      test_genome1.push_back('A');
   }

   std::string test_genome2 = "C";
   test_genome2.reserve(silo::GENOME_LENGTH);
   for (unsigned i = 1; i < silo::GENOME_LENGTH; ++i) {
      test_genome2.push_back('C');
   }

   sequence_store->interpret({std::move(test_genome1), std::move(test_genome2)});

   return sequence_store;
}

TEST(SequenceStore, shouldReturnCorrectBitmapForAmbiguousSymbol) {
  auto sequence_store = setupSequenceStore();

  // Note: 1-indexed!
  auto* bitmap2 = sequence_store->getBitmapFromAmbiguousSymbol(1, silo::NUCLEOTIDE_SYMBOL::A);
  EXPECT_NE(bitmap2, nullptr);
  EXPECT_TRUE(bitmap2->contains(0));
  EXPECT_FALSE(bitmap2->contains(1));
}

TEST(SequenceStore, shouldReturnCorrectFlippedBitmapForAmbiguousSymbol) {
  auto sequence_store = setupSequenceStore();

  // Note: 1-indexed!
  auto* bitmap = sequence_store->getFlippedBitmapFromAmbiguousSymbol(1, silo::NUCLEOTIDE_SYMBOL::A);
  EXPECT_NE(bitmap, nullptr);
  EXPECT_FALSE(bitmap->contains(0));
  EXPECT_TRUE(bitmap->contains(1));
}