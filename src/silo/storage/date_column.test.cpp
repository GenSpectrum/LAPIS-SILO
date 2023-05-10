#include "silo/storage/date_column.h"

#include <gtest/gtest.h>

using std::chrono::year;

TEST(RawDateColumn, filterShouldReturnRowsOfTheValue) {
   const std::string name = "test name";

   const silo::storage::RawDateColumn under_test(
      name,
      {
         {year{2022} / 12 / 1},
         {year{2022} / 12 / 2},
         {year{2022} / 12 / 3},
         {year{2022} / 12 / 1},
      }
   );

   const auto result1 = under_test.filter({year{2022} / 12 / 1});
   ASSERT_EQ(result1, roaring::Roaring({0, 3}));

   const auto result2 = under_test.filter({year{2022} / 12 / 2});
   ASSERT_EQ(result2, roaring::Roaring({1}));

   const auto result3 = under_test.filter({year{9999} / 12 / 31});
   ASSERT_EQ(result3, roaring::Roaring());
}

TEST(RawDateColumn, filterRangeShouldReturnRowsWithValueInRange) {
   const std::string name = "test name";

   const silo::storage::RawDateColumn under_test(
      name,
      {
         {year{2022} / 12 / 1},
         {year{2022} / 12 / 2},
         {year{2022} / 12 / 3},
         {year{2022} / 12 / 4},
         {year{2022} / 12 / 5},
         {year{2022} / 12 / 1},
         {year{2022} / 12 / 2},
         {year{2022} / 12 / 3},
         {year{2022} / 12 / 4},
         {year{2022} / 12 / 5},
      }
   );

   const auto result_from_equals_to =
      under_test.filterRange({year{2022} / 12 / 1}, {year{2022} / 12 / 1});
   ASSERT_EQ(result_from_equals_to, roaring::Roaring({0, 5}));

   const auto result_to_less_than_from =
      under_test.filterRange({year{2022} / 12 / 3}, {year{2022} / 12 / 1});
   ASSERT_EQ(result_to_less_than_from, roaring::Roaring());

   const auto result_from_less_than_to =
      under_test.filterRange({year{2022} / 12 / 2}, {year{2022} / 12 / 4});
   ASSERT_EQ(result_from_less_than_to, roaring::Roaring({1, 2, 3, 6, 7, 8}));

   const auto result_outside_of_values_range =
      under_test.filterRange({year{8888} / 1 / 1}, {year{9999} / 2 / 2});
   ASSERT_EQ(result_outside_of_values_range, roaring::Roaring());
}

TEST(SortedDateColumn, filterRangeShouldReturnRowsWithDuplicateValues) {
   const std::string name = "test name";

   const silo::storage::SortedDateColumn under_test(
      name,
      {
         {year{2022} / 12 / 1},
         {year{2022} / 12 / 1},
         {year{2022} / 12 / 2},
         {year{2022} / 12 / 2},
         {year{2022} / 12 / 3},
         {year{2022} / 12 / 3},
         {year{2022} / 12 / 4},
         {year{2022} / 12 / 4},
      }
   );

   const auto result_from_equals_to =
      under_test.filterRange({year{2022} / 12 / 2}, {year{2022} / 12 / 3});
   ASSERT_EQ(result_from_equals_to, roaring::Roaring({2, 3, 4, 5}));
}

using ColumnTypes = ::testing::Types<silo::storage::RawDateColumn, silo::storage::SortedDateColumn>;

template <typename T>
class DateColumnTest : public ::testing::Test {};

TYPED_TEST_SUITE(DateColumnTest, ColumnTypes);

TYPED_TEST(DateColumnTest, filterRangeShouldReturnRowsWithValueInRangeWhereValuesOverlapMonth) {
   const std::string name = "test name";

   const TypeParam under_test(
      name,
      {
         {year{2022} / 11 / 28},
         {year{2022} / 11 / 29},
         {year{2022} / 11 / 30},
         {year{2022} / 12 / 1},
         {year{2022} / 12 / 2},
         {year{2022} / 12 / 3},
         {year{2022} / 12 / 4},
      }
   );

   const auto result = under_test.filterRange({year{2022} / 11 / 29}, {year{2022} / 12 / 2});
   ASSERT_EQ(result, roaring::Roaring({1, 2, 3, 4}));
}

TYPED_TEST(DateColumnTest, filterRangeShouldReturnRowsWithValuesAreTrueSubsetOfFilterRange) {
   const std::string name = "test name";

   const TypeParam under_test(
      name,
      {
         {year{2022} / 11 / 29},
         {year{2022} / 11 / 30},
         {year{2022} / 12 / 1},
         {year{2022} / 12 / 2},
      }
   );

   const auto result = under_test.filterRange({year{2022} / 1 / 1}, {year{2022} / 12 / 31});
   ASSERT_EQ(result, roaring::Roaring({0, 1, 2, 3}));
}

TYPED_TEST(DateColumnTest, filterRangeShouldReturnRowsWithValueInRangeWhereValuesOverlapYear) {
   const std::string name = "test name";

   const TypeParam under_test(
      name,
      {
         {year{2022} / 12 / 29},
         {year{2022} / 12 / 30},
         {year{2022} / 12 / 31},
         {year{2023} / 1 / 1},
         {year{2023} / 1 / 2},
         {year{2023} / 1 / 3},
      }
   );

   const auto result = under_test.filterRange({year{2022} / 12 / 30}, {year{2023} / 1 / 2});
   ASSERT_EQ(result, roaring::Roaring({1, 2, 3, 4}));
}

TYPED_TEST(DateColumnTest, filterRangeShouldReturnRowsWithValueInRangeIncludingLeapDay) {
   const std::string name = "test name";

   const std::chrono::year_month_day leap_day = {year{2024} / 2 / 29};
   ASSERT_TRUE(leap_day.ok());

   const TypeParam under_test(
      name,
      {
         {year{2024} / 2 / 27},
         {year{2024} / 2 / 28},
         leap_day,
         {year{2024} / 3 / 1},
         {year{2024} / 3 / 2},
         {year{2024} / 3 / 3},
      }
   );

   const auto result = under_test.filterRange({year{2024} / 2 / 28}, {year{2024} / 3 / 2});
   ASSERT_EQ(result, roaring::Roaring({1, 2, 3, 4}));
}
