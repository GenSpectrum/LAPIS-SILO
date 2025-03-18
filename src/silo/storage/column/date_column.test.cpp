#include "silo/storage/column/date_column.h"

#include <gtest/gtest.h>

TEST(DateColumn, insertValues) {
   silo::storage::column::CM column_metadata{"test_column"};
   silo::storage::column::DateColumnPartition under_test(&column_metadata);

   under_test.insert("2020-01-01");
   under_test.insert("2023-1-05");
   under_test.insert("2021-12-03");
   under_test.insert("2025-01-01");
   under_test.insert("2021-03-21");
   under_test.insert("");
}
