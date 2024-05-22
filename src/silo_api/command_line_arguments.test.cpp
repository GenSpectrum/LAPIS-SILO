#include "silo_api/command_line_arguments.h"

#include <gtest/gtest.h>

TEST(CommandLineArguments, correctUnixOptionString) {
   ASSERT_EQ(silo_api::CommandLineArguments::asUnixOptionString({{""}}), "");
   ASSERT_EQ(silo_api::CommandLineArguments::asUnixOptionString({{"A"}}), "-a");
   ASSERT_EQ(silo_api::CommandLineArguments::asUnixOptionString({{"abc"}}), "abc");
   ASSERT_EQ(
      silo_api::CommandLineArguments::asUnixOptionString({{"someCamelCase"}}), "some-camel-case"
   );
   ASSERT_EQ(
      silo_api::CommandLineArguments::asUnixOptionString({{"BADCamelCase"}}), "-b-a-d-camel-case"
   );
   ASSERT_EQ(
      silo_api::CommandLineArguments::asUnixOptionString({{"something_with_underscores"}}),
      "something_with_underscores"
   );
   ASSERT_EQ(
      silo_api::CommandLineArguments::asUnixOptionString({{"some", "subsectionedSequence"}}),
      "some-subsectioned-sequence"
   );
   ASSERT_EQ(
      silo_api::CommandLineArguments::asUnixOptionString({{"some", "more", "sections"}}),
      "some-more-sections"
   );
}