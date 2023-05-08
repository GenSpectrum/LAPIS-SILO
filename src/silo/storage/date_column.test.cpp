#include "silo/storage/date_column.h"

#include <gtest/gtest.h>

using std::chrono::day;
using std::chrono::month;
using std::chrono::year;

TEST(DateColumn, shouldReturnTheCorrectFilteredValues) {
   const std::string name = "test name";

   const silo::storage::DateColumn under_test(
      name,
      {
         {year{2022}, month{12}, day{1}},
         {year{2022}, month{12}, day{2}},
         {year{2022}, month{12}, day{3}},
         {year{2022}, month{12}, day{1}},
      }
   );

   const auto result1 = under_test.filter({year{2022}, month{12}, day{1}});
   ASSERT_EQ(result1, roaring::Roaring({0, 3}));

   const auto result2 = under_test.filter({year{2022}, month{12}, day{2}});
   ASSERT_EQ(result2, roaring::Roaring({1}));

   const auto result3 = under_test.filter({year{9999}, month{12}, day{31}});
   ASSERT_EQ(result3, roaring::Roaring());
}
