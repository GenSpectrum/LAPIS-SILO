#include "silo/common/date.h"

#include <gtest/gtest.h>

#include "silo/common/date_format_exception.h"

TEST(Date, correctlyParsesDate) {
   EXPECT_EQ(silo::common::stringToDate("2020-01-01"), (2020 << 16) + (1 << 12) + 1);
   EXPECT_EQ(silo::common::stringToDate("2023-1-01"), (2023 << 16) + (1 << 12) + 1);
   EXPECT_EQ(silo::common::stringToDate("2010-12-03"), (2010 << 16) + (12 << 12) + 3);
   EXPECT_EQ(silo::common::stringToDate("12-12-12"), (12 << 16) + (12 << 12) + 12);
}

TEST(Date, throwsExceptionOnWrongDates) {
   EXPECT_THROW(silo::common::stringToDate(""), silo::common::DateFormatException);
   EXPECT_THROW(silo::common::stringToDate("----"), silo::common::DateFormatException);
   EXPECT_THROW(silo::common::stringToDate("31-31-31"), silo::common::DateFormatException);
   EXPECT_THROW(silo::common::stringToDate("-1-"), silo::common::DateFormatException);
   EXPECT_THROW(
      silo::common::stringToDate("31-31123123-31123123123123412"), silo::common::DateFormatException
   );
   EXPECT_THROW(silo::common::stringToDate("31-0-1"), silo::common::DateFormatException);
   EXPECT_THROW(silo::common::stringToDate("31-31-31"), silo::common::DateFormatException);
}

TEST(Date, correctlyReprintsStrings) {
   EXPECT_EQ(silo::common::dateToString(silo::common::stringToDate("12-12-12")), "0012-12-12");
   EXPECT_EQ(silo::common::dateToString(silo::common::stringToDate("2020-01-01")), "2020-01-01");
   EXPECT_EQ(silo::common::dateToString(silo::common::stringToDate("2023-01-1")), "2023-01-01");
   EXPECT_EQ(silo::common::dateToString(silo::common::stringToDate("2010-12-3")), "2010-12-03");
}
