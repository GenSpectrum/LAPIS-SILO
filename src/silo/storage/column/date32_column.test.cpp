#include "silo/storage/column/date32_column.h"

#include <gtest/gtest.h>

#include "silo/common/date32.h"

TEST(Date32Column, insertValues) {
   silo::storage::column::ColumnMetadata column_metadata{"test_column"};
   silo::storage::column::Date32Column under_test(&column_metadata);

   std::vector<std::string> values_to_add{
      "2020-01-01", "2023-01-05", "2021-12-03", "2025-01-01", "2021-03-21"
   };
   silo::storage::column::Date32Column::Builder builder;
   for (const auto& value : values_to_add) {
      auto date = silo::common::stringToDate32(value);
      ASSERT_TRUE(date.has_value()) << date.error();
      builder.insert(date.value());
   }
   SILO_ASSERT(under_test.appendChunk(builder.finalize()).has_value());

   ASSERT_EQ(under_test.getValues().size(), 5);

   for (size_t value_idx = 0; value_idx < values_to_add.size(); ++value_idx) {
      auto value = silo::common::date32ToString(under_test.getValues().at(value_idx));
      ASSERT_EQ(value, values_to_add.at(value_idx));
   }
}

TEST(Date32Column, insertNull) {
   silo::storage::column::ColumnMetadata column_metadata{"test_column"};
   silo::storage::column::Date32Column under_test(&column_metadata);

   silo::storage::column::Date32Column::Builder builder;
   builder.insertNull();
   SILO_ASSERT(under_test.appendChunk(builder.finalize()).has_value());

   ASSERT_EQ(under_test.numValues(), 1);
   ASSERT_TRUE(under_test.isNull(0));
}

TEST(Date32Column, parseInvalidDateReturnsError) {
   // Date parsing happens during phase-1 extraction (see stringToDate32), before
   // values reach the column builder.
   auto result = silo::common::stringToDate32("not-a-date");
   ASSERT_FALSE(result.has_value());
}
