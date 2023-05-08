#include "silo/storage/string_column.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <silo/storage/dictionary.h>

class MockDictionary : public silo::Dictionary {
  public:
   MOCK_METHOD(
      (std::optional<uint32_t>),
      lookupValueId,
      (const std::string& column_name, const std::string& value),
      (const)
   );
};

TEST(RawStringColumn, shouldReturnTheCorrectFilteredValues) {
   const std::string name = "test name";

   const silo::storage::RawStringColumn under_test(
      name, {"value 1", "value 2", "value 3", "value 1"}
   );

   const auto result1 = under_test.filter("value 1");
   ASSERT_EQ(result1, roaring::Roaring({0, 3}));

   const auto result2 = under_test.filter("value 2");
   ASSERT_EQ(result2, roaring::Roaring({1}));

   const auto result3 = under_test.filter("value that does not exist");
   ASSERT_EQ(result3, roaring::Roaring());
}

TEST(IndexedStringColumn, shouldReturnTheCorrectFilteredValues) {
   const std::string column_name = "test name";

   const auto dictionary_mock = MockDictionary();

   EXPECT_CALL(dictionary_mock, lookupValueId(testing::Eq(column_name), testing::Eq("value 1")))
      .WillRepeatedly(testing::Return(0));
   EXPECT_CALL(dictionary_mock, lookupValueId(testing::Eq(column_name), testing::Eq("value 2")))
      .WillRepeatedly(testing::Return(1));
   EXPECT_CALL(
      dictionary_mock,
      lookupValueId(testing::Eq(column_name), testing::Eq("value that does not exist"))
   )
      .WillRepeatedly(testing::Return(std::nullopt));

   const silo::storage::IndexedStringColumn under_test(
      column_name, dictionary_mock, {{0}, {1, 2}, {3}}
   );

   const auto result1 = under_test.filter("value 1");
   ASSERT_EQ(result1, roaring::Roaring({0}));

   const auto result2 = under_test.filter("value 2");
   ASSERT_EQ(result2, roaring::Roaring({1, 2}));

   const auto result3 = under_test.filter("value that does not exist");
   ASSERT_EQ(result3, roaring::Roaring());
}
