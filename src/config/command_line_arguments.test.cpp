#include "config/source/command_line_arguments.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "silo/config/util/config_exception.h"

using silo::config::CommandLineArguments;
using silo::config::ConfigException;
using silo::config::ConfigKeyPath;

TEST(CommandLineArguments, correctUnixOptionString) {
   ASSERT_EQ(
      CommandLineArguments::configKeyPathToString(ConfigKeyPath::tryFrom({{"a"}}).value()), "--a"
   );
   ASSERT_EQ(
      CommandLineArguments::configKeyPathToString(ConfigKeyPath::tryFrom({{"abc"}}).value()),
      "--abc"
   );
   ASSERT_EQ(
      CommandLineArguments::configKeyPathToString(ConfigKeyPath::tryFrom({{"ab2c"}}).value()),
      "--ab2c"
   );
   ASSERT_EQ(
      CommandLineArguments::configKeyPathToString(ConfigKeyPath::tryFrom({{"some", "camel", "case"}}
      ).value()),
      "--some-camel-case"
   );
   ASSERT_EQ(
      CommandLineArguments::configKeyPathToString(
         ConfigKeyPath::tryFrom({{"some"}, {"subsectioned", "sequence"}}).value()
      ),
      "--some-subsectioned-sequence"
   );
   ASSERT_EQ(
      CommandLineArguments::configKeyPathToString(
         ConfigKeyPath::tryFrom({{"some", "more", "sections"}}).value()
      ),
      "--some-more-sections"
   );
}

TEST(CommandLineArguments, shouldFailAppropriatelyOnInvalidOptions) {
   const std::vector<std::string> invalid_options{
      "no-starting-minus",
      "---too-many-starting-minuses",
      "--NOT-LOWER-CASE",
      "--not&alphanumeric",
      "",
      "-",
      "--",
      "&",
      "\n",
      "--wrong_delimiter"
   };
   for (const auto& invalid_option : invalid_options) {
      EXPECT_THAT(
         [&invalid_option]() { CommandLineArguments::stringToConfigKeyPath(invalid_option); },
         ThrowsMessage<ConfigException>(::testing::HasSubstr(fmt::format(
            "the provided option '{}' is not a valid command line option", invalid_option
         )))
      );
   }
}
