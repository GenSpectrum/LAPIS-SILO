#include "silo/storage/sequence_store.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <memory>
#include <string>

constexpr size_t genome_length = 29903;

std::vector<std::string> buildTwoDistinctGenomes() {
   std::string test_genome1;
   std::fill_n(std::back_inserter(test_genome1), genome_length, 'A');
   std::string test_genome2;
   std::fill_n(std::back_inserter(test_genome2), genome_length, 'C');
   return {std::move(test_genome1), std::move(test_genome2)};
}

std::unique_ptr<silo::SequenceStorePartition> setupSequenceStore() {
   std::string reference_genome;
   std::fill_n(std::back_inserter(reference_genome), genome_length, 'G');
   auto sequence_store = std::make_unique<silo::SequenceStorePartition>(reference_genome);
   sequence_store->interpret(buildTwoDistinctGenomes());
   return sequence_store;
}
