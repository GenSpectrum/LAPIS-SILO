#include "silo/common/date.h"

#include <gtest/gtest.h>

TEST(Date, correctlyParsesDate) {
   EXPECT_EQ(silo::common::stringToDate("2020-01-01").value(), 18262);
   EXPECT_EQ(silo::common::stringToDate("1970-01-01").value(), 0);
   EXPECT_EQ(silo::common::stringToDate("2010-12-03").value(), 14946);
   EXPECT_EQ(silo::common::stringToDate("1969-12-31").value(), -1);
}

TEST(Date, parsesPreEpochDates) {
   EXPECT_TRUE(silo::common::stringToDate("1900-01-01").has_value());
   EXPECT_LT(silo::common::stringToDate("1900-01-01").value(), 0);
   EXPECT_EQ(silo::common::stringToDate("1969-12-31").value(), -1);
   EXPECT_EQ(silo::common::stringToDate("1969-12-30").value(), -2);
}

TEST(Date, parsesLeapYearDates) {
   EXPECT_TRUE(silo::common::stringToDate("2024-02-29").has_value());
   EXPECT_TRUE(silo::common::stringToDate("2000-02-29").has_value());
   EXPECT_FALSE(silo::common::stringToDate("2023-02-29").has_value());
   EXPECT_FALSE(silo::common::stringToDate("1900-02-29").has_value());
   EXPECT_FALSE(silo::common::stringToDate("2100-02-29").has_value());
}

TEST(Date, parsesBoundaryDaysPerMonth) {
   EXPECT_TRUE(silo::common::stringToDate("2023-01-31").has_value());
   EXPECT_FALSE(silo::common::stringToDate("2023-01-32").has_value());
   EXPECT_TRUE(silo::common::stringToDate("2023-03-31").has_value());
   EXPECT_FALSE(silo::common::stringToDate("2023-03-32").has_value());
   EXPECT_TRUE(silo::common::stringToDate("2023-04-30").has_value());
   EXPECT_FALSE(silo::common::stringToDate("2023-04-31").has_value());
   EXPECT_TRUE(silo::common::stringToDate("2023-02-28").has_value());
   EXPECT_FALSE(silo::common::stringToDate("2023-02-29").has_value());
}

TEST(Date, rejectsWrongFormat) {
   EXPECT_FALSE(silo::common::stringToDate("").has_value());
   EXPECT_FALSE(silo::common::stringToDate("?").has_value());
   EXPECT_FALSE(silo::common::stringToDate("----").has_value());
   EXPECT_FALSE(silo::common::stringToDate("2023-1-01").has_value());
   EXPECT_FALSE(silo::common::stringToDate("12-12-12").has_value());
   EXPECT_FALSE(silo::common::stringToDate("-1-").has_value());
}

TEST(Date, rejectsWrongSeparators) {
   EXPECT_FALSE(silo::common::stringToDate("2023/01/15").has_value());
   EXPECT_FALSE(silo::common::stringToDate("2023.01.15").has_value());
   EXPECT_FALSE(silo::common::stringToDate("2023 01 15").has_value());
}

TEST(Date, rejectsWrongFieldWidths) {
   EXPECT_FALSE(silo::common::stringToDate("23-01-15").has_value());
   EXPECT_FALSE(silo::common::stringToDate("2023-1-15").has_value());
   EXPECT_FALSE(silo::common::stringToDate("2023-01-1").has_value());
   EXPECT_FALSE(silo::common::stringToDate("20230-01-15").has_value());
}

TEST(Date, rejectsNonNumericFields) {
   EXPECT_FALSE(silo::common::stringToDate("abcd-01-15").has_value());
   EXPECT_FALSE(silo::common::stringToDate("2023-ab-15").has_value());
   EXPECT_FALSE(silo::common::stringToDate("2023-01-ab").has_value());
}

TEST(Date, rejectsTrailingAndLeadingContent) {
   EXPECT_FALSE(silo::common::stringToDate("2023-01-15 ").has_value());
   EXPECT_FALSE(silo::common::stringToDate(" 2023-01-15").has_value());
   EXPECT_FALSE(silo::common::stringToDate("2023-01-15T00:00:00").has_value());
}

TEST(Date, rejectsInvalidCalendarDates) {
   EXPECT_FALSE(silo::common::stringToDate("2023-02-30").has_value());
   EXPECT_FALSE(silo::common::stringToDate("2023-13-01").has_value());
   EXPECT_FALSE(silo::common::stringToDate("2023-00-01").has_value());
   EXPECT_FALSE(silo::common::stringToDate("2023-01-00").has_value());
}

TEST(Date, errorMessagesAreDescriptive) {
   auto format_error = silo::common::stringToDate("not-a-date");
   ASSERT_FALSE(format_error.has_value());
   EXPECT_NE(format_error.error().find("not-a-date"), std::string::npos);

   auto calendar_error = silo::common::stringToDate("2023-02-30");
   ASSERT_FALSE(calendar_error.has_value());
   EXPECT_NE(calendar_error.error().find("2023-02-30"), std::string::npos);
}

TEST(Date, correctlyRoundTrips) {
   const std::vector<std::string> dates = {
      "2020-01-01",
      "2010-12-03",
      "1970-01-01",
      "1969-12-31",
      "2000-02-29",
      "2024-12-31",
      "1900-01-01",
      "2099-06-15",
   };
   for (const auto& date_string : dates) {
      auto parsed = silo::common::stringToDate(date_string);
      ASSERT_TRUE(parsed.has_value()) << "Failed to parse: " << date_string;
      EXPECT_EQ(silo::common::dateToString(parsed.value()), date_string);
   }
}

TEST(Date, dateToStringFormatsWithLeadingZeros) {
   EXPECT_EQ(silo::common::dateToString(0), "1970-01-01");
   EXPECT_EQ(silo::common::dateToString(-1), "1969-12-31");
}

TEST(Date, epochIsZero) {
   EXPECT_EQ(silo::common::stringToDate("1970-01-01").value(), 0);
}

TEST(Date, daysAreMonotonicallyIncreasing) {
   auto jan1 = silo::common::stringToDate("2023-01-01").value();
   auto jan2 = silo::common::stringToDate("2023-01-02").value();
   auto feb1 = silo::common::stringToDate("2023-02-01").value();
   auto dec31 = silo::common::stringToDate("2023-12-31").value();
   EXPECT_EQ(jan2 - jan1, 1);
   EXPECT_LT(jan1, feb1);
   EXPECT_LT(feb1, dec31);
}

TEST(Date, yearBoundaryIsOneDay) {
   auto dec31 = silo::common::stringToDate("2022-12-31").value();
   auto jan1 = silo::common::stringToDate("2023-01-01").value();
   EXPECT_EQ(jan1 - dec31, 1);
}
