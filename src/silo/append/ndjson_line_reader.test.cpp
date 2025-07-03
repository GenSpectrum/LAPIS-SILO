#include "silo/append/ndjson_line_reader.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using silo::append::AppendException;
using silo::append::NdjsonLineReader;

TEST(NdjsonLineReader, throwsAppendErrorOnInvalidJson) {
   std::string invalid_json = "{}\n{";
   std::stringstream invalid_json_stream{invalid_json};
   NdjsonLineReader reader{invalid_json_stream};
   EXPECT_THAT(
      [&]() { (void)reader.begin()++; },
      ThrowsMessage<silo::append::AppendException>(
         ::testing::HasSubstr("Error while parsing ndjson file")
      )
   );
}

TEST(NdjsonLineReader, throwsAppendErrorOnInvalidJsonString) {
   std::string invalid_json = "{}\n{\"test\":\"}";
   std::stringstream invalid_json_stream{invalid_json};
   NdjsonLineReader reader{invalid_json_stream};
   EXPECT_THAT(
      [&]() { (void)reader.begin()++; },
      ThrowsMessage<silo::append::AppendException>(
         ::testing::HasSubstr("Error while parsing ndjson file")
      )
   );
}
