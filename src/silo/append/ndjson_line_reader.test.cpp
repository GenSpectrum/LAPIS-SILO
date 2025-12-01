#include "silo/append/ndjson_line_reader.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using silo::append::NdjsonLineReader;

TEST(NdjsonLineReader, returnsErrorResultOnInvalidLines) {
   std::string invalid_json = "{}\n{";
   std::stringstream invalid_json_stream{invalid_json};
   NdjsonLineReader reader{invalid_json_stream};
   auto iter = reader.begin();
   auto [first_object, first_line] = *iter;
   ASSERT_FALSE(first_object.error());
   ASSERT_EQ(first_line, "{}");
   auto [second_object, second_line] = *(++iter);
   ASSERT_EQ(
      second_object.value_unsafe().get_object().error(), simdjson::INCOMPLETE_ARRAY_OR_OBJECT
   );
   ASSERT_EQ(second_line, "{");
}

TEST(NdjsonLineReader, throwsAppendErrorOnInvalidJsonString) {
   std::string invalid_json = "{}\n{\"test\":\"}\n{}";
   std::stringstream invalid_json_stream{invalid_json};
   NdjsonLineReader reader{invalid_json_stream};
   auto iter = reader.begin();
   auto [first_object, first_line] = *iter;
   ASSERT_FALSE(first_object.error());
   auto [second_object, second_line] = *(++iter);
   ASSERT_TRUE(second_object.error());
}

TEST(NdjsonLineReader, validOnEmptyString) {
   std::string invalid_json;
   std::stringstream invalid_json_stream{invalid_json};
   NdjsonLineReader reader{invalid_json_stream};
   ASSERT_TRUE(reader.begin() == reader.end());
}

TEST(NdjsonLineReader, validOnNoNewLine) {
   std::string invalid_json = "{}";
   std::stringstream invalid_json_stream{invalid_json};
   NdjsonLineReader reader{invalid_json_stream};
   auto iter = reader.begin();
   ASSERT_TRUE(iter != reader.end());
   auto [first_object, first_line] = *iter;
   ASSERT_FALSE(first_object.error());
   ++iter;
   ASSERT_TRUE(iter == reader.end());
}

TEST(NdjsonLineReader, validOnTerminatedLine) {
   std::string invalid_json = "{}\n";
   std::stringstream invalid_json_stream{invalid_json};
   NdjsonLineReader reader{invalid_json_stream};
   auto iter = reader.begin();
   ASSERT_TRUE(iter != reader.end());
   auto [first_object, first_line] = *iter;
   ASSERT_FALSE(first_object.error());
   ++iter;
   ASSERT_TRUE(iter == reader.end());
}
