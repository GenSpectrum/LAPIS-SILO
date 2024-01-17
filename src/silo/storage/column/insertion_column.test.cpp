#include "silo/storage/column/insertion_column.h"

#include <gtest/gtest.h>

using silo::storage::column::InsertionColumnPartition;

TEST(InsertionColumn, insertValuesToPartition) {
   silo::common::BidirectionalMap<std::string> lookup;
   const std::optional<std::string> default_name = "main";
   InsertionColumnPartition<silo::Nucleotide> under_test(lookup, default_name);

   under_test.insert("25701:ACCA");
   under_test.insert("2301:CCG");
   under_test.insert("2301:CCG");
   under_test.insert("19832:TTACA");
   under_test.insert("25701:ACCA");

   under_test.buildInsertionIndexes();

   EXPECT_EQ(under_test.getValues()[0], 0U);
   EXPECT_EQ(under_test.getValues()[1], 1U);
   EXPECT_EQ(under_test.getValues()[2], 1U);
   EXPECT_EQ(under_test.getValues()[3], 2U);
   EXPECT_EQ(under_test.getValues()[4], 0U);

   EXPECT_EQ(under_test.lookupValue(0U), "25701:ACCA");
   EXPECT_EQ(under_test.lookupValue(1U), "2301:CCG");
   EXPECT_EQ(under_test.lookupValue(2U), "19832:TTACA");
}

TEST(InsertionColumn, shouldReturnTheCorrectSearchedValues) {
   silo::common::BidirectionalMap<std::string> lookup;
   const std::optional<std::string> default_name = "main";
   InsertionColumnPartition<silo::Nucleotide> under_test(lookup, default_name);

   under_test.insert("25701:ACCA");
   under_test.insert("25701:CCG");
   under_test.insert("25701:CCG");
   under_test.insert("25701:TTACAT,25701:ACCA");
   under_test.insert("25701:ACCA");
   under_test.insert("25701:TTACAT,25701:ACCA,25701:AGCTGTTCAG");

   under_test.buildInsertionIndexes();

   const auto result1 = under_test.search("main", 25701, ".*CC.*");
   ASSERT_EQ(*result1, roaring::Roaring({0, 1, 2, 3, 4, 5}));

   const auto result2 = under_test.search("main", 25701, ".*TTA.*CAT.*");
   ASSERT_EQ(*result2, roaring::Roaring({3, 5}));

   const auto result3 = under_test.search("main", 25701, ".*AGC.*TGT.*TCA.*G.*");
   ASSERT_EQ(*result3, roaring::Roaring({5}));

   const auto result4 = under_test.search("main", 25701, ".*AGC.*TG.*T.*T.*C.*AG.*");
   ASSERT_EQ(*result4, roaring::Roaring({5}));

   const auto result5 = under_test.search("main", 25701, ".*TTT.*AAA.*");
   ASSERT_EQ(*result5, roaring::Roaring());

   const auto result6 = under_test.search("main", 100, ".*TTT.*AAA.*");
   ASSERT_EQ(*result5, roaring::Roaring());
}
