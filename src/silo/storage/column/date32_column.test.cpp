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

   ASSERT_EQ(under_test.numValues(), 5);

   for (size_t value_idx = 0; value_idx < values_to_add.size(); ++value_idx) {
      auto value = silo::common::date32ToString(under_test.getValue(value_idx));
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

namespace {
silo::common::Date32 date(const std::string& value) {
   const auto parsed = silo::common::stringToDate32(value);
   SILO_ASSERT(parsed.has_value());
   return parsed.value();
}

silo::storage::column::Date32Column::Buffer chunkOf(const std::vector<std::string>& values) {
   silo::storage::column::Date32Column::Builder builder;
   for (const auto& value : values) {
      if (value.empty()) {
         builder.insertNull();
      } else {
         builder.insert(date(value));
      }
   }
   return builder.finalize();
}
}  // namespace

TEST(Date32Column, staysSortedWhenChunkBoundaryIsGloballySorted) {
   silo::storage::column::ColumnMetadata column_metadata{"test_column"};
   silo::storage::column::Date32Column under_test(&column_metadata);

   // Each chunk is sorted on its own, and the first value of every later chunk is >= the last
   // value of the previous chunk, so the column is globally sorted across boundaries.
   SILO_ASSERT(under_test.appendChunk(chunkOf({"2020-01-01", "2020-06-01"})).has_value());
   SILO_ASSERT(under_test.appendChunk(chunkOf({"2020-06-01", "2021-03-21"})).has_value());
   SILO_ASSERT(under_test.appendChunk(chunkOf({"2022-01-01", "2025-12-31"})).has_value());

   ASSERT_EQ(under_test.numValues(), 6);
   ASSERT_TRUE(under_test.isSorted());
}

TEST(Date32Column, notSortedWhenChunkBoundaryRegresses) {
   silo::storage::column::ColumnMetadata column_metadata{"test_column"};
   silo::storage::column::Date32Column under_test(&column_metadata);

   // Each chunk is sorted in isolation, but the second chunk starts before the first chunk ends.
   // Detecting this requires the cross-chunk ingestion state (last_appended_value); within-chunk
   // checks alone would wrongly report the column as sorted.
   SILO_ASSERT(under_test.appendChunk(chunkOf({"2020-01-01", "2023-01-05"})).has_value());
   SILO_ASSERT(under_test.appendChunk(chunkOf({"2021-12-03", "2025-01-01"})).has_value());

   ASSERT_EQ(under_test.numValues(), 4);
   ASSERT_FALSE(under_test.isSorted());
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST(Date32Column, nullBitmapRowIdsAreOffsetAcrossChunks) {
   silo::storage::column::ColumnMetadata column_metadata{"test_column"};
   silo::storage::column::Date32Column under_test(&column_metadata);

   // Nulls appear in every chunk, including at the start of a chunk that is not the first one,
   // so a missing per-chunk offset would land the null bitmap on the wrong global row ids.
   SILO_ASSERT(under_test.appendChunk(chunkOf({"2020-01-01", ""})).has_value());      // rows 0,1
   SILO_ASSERT(under_test.appendChunk(chunkOf({"", "2021-03-21", ""})).has_value());  // rows 2,3,4
   SILO_ASSERT(under_test.appendChunk(chunkOf({"2022-01-01"})).has_value());          // row 5

   ASSERT_EQ(under_test.numValues(), 6);

   const std::vector<bool> expected_null{false, true, true, false, true, false};
   for (size_t row_id = 0; row_id < expected_null.size(); ++row_id) {
      ASSERT_EQ(under_test.isNull(row_id), expected_null[row_id]) << "row_id=" << row_id;
   }

   ASSERT_FALSE(under_test.isNull(0));
   ASSERT_EQ(silo::common::date32ToString(under_test.getValue(3)), "2021-03-21");
   ASSERT_EQ(silo::common::date32ToString(under_test.getValue(5)), "2022-01-01");
}
