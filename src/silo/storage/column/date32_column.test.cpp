#include "silo/storage/column/date32_column.h"

#include <gtest/gtest.h>

TEST(Date32ColumnPartition, insertValues) {
   silo::storage::column::ColumnMetadata column_metadata{"test_column"};
   silo::storage::column::Date32ColumnPartition under_test(&column_metadata);

   std::vector<std::string> values_to_add{
      "2020-01-01", "2023-01-05", "2021-12-03", "2025-01-01", "2021-03-21"
   };
   for (const auto& value : values_to_add) {
      auto result = under_test.insert(value);
      ASSERT_TRUE(result.has_value()) << result.error();
   }

   ASSERT_EQ(under_test.getValues().size(), 5);

   for (size_t value_idx = 0; value_idx < values_to_add.size(); ++value_idx) {
      auto value = silo::common::date32ToString(under_test.getValues().at(value_idx));
      ASSERT_EQ(value, values_to_add.at(value_idx));
   }
}

TEST(Date32ColumnPartition, insertNull) {
   silo::storage::column::ColumnMetadata column_metadata{"test_column"};
   silo::storage::column::Date32ColumnPartition under_test(&column_metadata);

   under_test.insertNull();

   ASSERT_EQ(under_test.numValues(), 1);
   ASSERT_TRUE(under_test.isNull(0));
}

TEST(Date32ColumnPartition, insertInvalidDateReturnsError) {
   silo::storage::column::ColumnMetadata column_metadata{"test_column"};
   silo::storage::column::Date32ColumnPartition under_test(&column_metadata);

   auto result = under_test.insert("not-a-date");
   ASSERT_FALSE(result.has_value());
   ASSERT_EQ(under_test.numValues(), 0);
}
