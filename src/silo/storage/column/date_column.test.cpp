#include "silo/storage/column/date_column.h"

#include <gtest/gtest.h>

TEST(DateColumn, insertValues) {
   silo::storage::column::DateColumnPartition under_test(false);

   under_test.insert(silo::common::stringToDate("2020-01-01"));
   under_test.insert(silo::common::stringToDate("2023-1-05"));
   under_test.insert(silo::common::stringToDate("2021-12-03"));
   under_test.insert(silo::common::stringToDate("2025-01-01"));
   under_test.insert(silo::common::stringToDate("2021-03-21"));
   under_test.insert(silo::common::stringToDate(""));
}
