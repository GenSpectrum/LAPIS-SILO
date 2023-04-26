#include "silo/storage/sequence_store.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <iterator>
#include <memory>
#include <string>

std::vector<std::string> buildTwoDistinctGenomes() {
   std::string test_genome1;
   std::fill_n(std::back_inserter(test_genome1), silo::GENOME_LENGTH, 'A');
   std::string test_genome2;
   std::fill_n(std::back_inserter(test_genome2), silo::GENOME_LENGTH, 'C');
   return {std::move(test_genome1), std::move(test_genome2)};
}

std::unique_ptr<silo::SequenceStore> setupSequenceStore() {
   std::unique_ptr<silo::SequenceStore> sequence_store = std::make_unique<silo::SequenceStore>();
   sequence_store->interpret(buildTwoDistinctGenomes());
   return sequence_store;
}

TEST(SequenceStore, shouldReturnCorrectBitmapForAmbiguousSymbol) {
   auto sequence_store = setupSequenceStore();
   auto* bitmap = sequence_store->getBitmapFromAmbiguousSymbol(1, silo::NUCLEOTIDE_SYMBOL::A);
   EXPECT_NE(bitmap, nullptr);
   EXPECT_TRUE(bitmap->contains(0));
   EXPECT_FALSE(bitmap->contains(1));
}

TEST(SequenceStore, shouldReturnCorrectFlippedBitmapForAmbiguousSymbol) {
   auto sequence_store = setupSequenceStore();
   auto* bitmap =
      sequence_store->getFlippedBitmapFromAmbiguousSymbol(1, silo::NUCLEOTIDE_SYMBOL::A);
   EXPECT_NE(bitmap, nullptr);
   EXPECT_FALSE(bitmap->contains(0));
   EXPECT_TRUE(bitmap->contains(1));
}