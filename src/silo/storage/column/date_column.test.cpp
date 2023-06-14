#include "silo/storage/column/date_column.h"

#include <gtest/gtest.h>

TEST(DateColumn, insertedValuesRequeried) {
   silo::storage::column::DateColumn under_test;

   under_test.insert(silo::common::stringToDate("2020-01-01"));
   under_test.insert(silo::common::stringToDate("2023-1-05"));
   under_test.insert(silo::common::stringToDate("2021-12-03"));
   under_test.insert(silo::common::stringToDate("2025-01-01"));
   under_test.insert(silo::common::stringToDate("2021-03-21"));

   EXPECT_EQ(under_test.getAsString(0U), "2020-01-01");
   EXPECT_EQ(under_test.getAsString(1U), "2023-01-05");
   EXPECT_EQ(under_test.getAsString(2U), "2021-12-03");
   EXPECT_EQ(under_test.getAsString(3U), "2025-01-01");
   EXPECT_EQ(under_test.getAsString(4U), "2021-03-21");
}
