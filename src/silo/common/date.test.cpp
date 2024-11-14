#include "silo/common/date.h"

#include <gtest/gtest.h>

TEST(Date, correctlyParsesDate) {
   EXPECT_EQ(silo::common::stringToDate("2020-01-01"), (2020 << 16) + (1 << 12) + 1);
   EXPECT_EQ(silo::common::stringToDate("2023-1-01"), (2023 << 16) + (1 << 12) + 1);
   EXPECT_EQ(silo::common::stringToDate("2010-12-03"), (2010 << 16) + (12 << 12) + 3);
   EXPECT_EQ(silo::common::stringToDate("12-12-12"), (12 << 16) + (12 << 12) + 12);
   EXPECT_EQ(silo::common::stringToDate(""), 0);
}

TEST(Date, ignoresWrongDates) {
   EXPECT_EQ(silo::common::stringToDate("?"), 0);
   EXPECT_EQ(silo::common::stringToDate("----"), 0);
   EXPECT_EQ(silo::common::stringToDate("31-31-31"), 0);
   EXPECT_EQ(silo::common::stringToDate("-1-"), 0);
   EXPECT_EQ(silo::common::stringToDate("31-31123123-31123123123123412"), 0);
   EXPECT_EQ(silo::common::stringToDate("31-0-1"), 0);
   EXPECT_EQ(silo::common::stringToDate("31-31-31"), 0);
}

TEST(Date, correctlyReprintsStrings) {
   EXPECT_EQ(silo::common::dateToString(silo::common::stringToDate("12-12-12")), "0012-12-12");
   EXPECT_EQ(silo::common::dateToString(silo::common::stringToDate("2020-01-01")), "2020-01-01");
   EXPECT_EQ(silo::common::dateToString(silo::common::stringToDate("2023-01-1")), "2023-01-01");
   EXPECT_EQ(silo::common::dateToString(silo::common::stringToDate("2010-12-3")), "2010-12-03");

   EXPECT_EQ(silo::common::dateToString(silo::common::stringToDate("")), std::nullopt);
}
