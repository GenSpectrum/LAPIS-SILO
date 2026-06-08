#include "config/source/environment_variables.h"

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

using silo::config::ConfigKeyPath;
using silo::config::EnvironmentVariables;

TEST(EnvironmentVariables, correctPrefixedUppercase) {
   ASSERT_EQ(
      EnvironmentVariables::configKeyPathToString(ConfigKeyPath::tryFrom({{"a"}}).value()), "SILO_A"
   );
   ASSERT_EQ(
      EnvironmentVariables::configKeyPathToString(ConfigKeyPath::tryFrom({{"abc"}}).value()),
      "SILO_ABC"
   );
   ASSERT_EQ(
      EnvironmentVariables::configKeyPathToString(ConfigKeyPath::tryFrom({{"some", "snake", "case"}}
      ).value()),
      "SILO_SOME_SNAKE_CASE"
   );
   ASSERT_EQ(
      EnvironmentVariables::configKeyPathToString(
         ConfigKeyPath::tryFrom({{"some"}, {"subsectioned", "sequence"}}).value()
      ),
      "SILO_SOME_SUBSECTIONED_SEQUENCE"
   );
   ASSERT_EQ(
      EnvironmentVariables::configKeyPathToString(
         ConfigKeyPath::tryFrom({{"some"}, {"more"}, {"sections"}}).value()
      ),
      "SILO_SOME_MORE_SECTIONS"
   );
}

TEST(EnvironmentVariables, successfullyIgnoreTheAllowList) {
   const std::vector<std::string> allow_list = {"SILO_DEBUG"};
   const char* env_var = "SILO_DEBUG=1";
   const std::vector<const char*> var_vector = {env_var, nullptr};
   auto env_vars = EnvironmentVariables::newWithAllowListAndEnv(allow_list, var_vector.data());
   ASSERT_NO_THROW((void)env_vars.verify({}));
}

TEST(EnvironmentVariables, errorsIfSiloDebugIsProvidedButNotAllowed) {
   const std::vector<std::string> allow_list;
   const char* env_var = "SILO_DEBUG=1";
   const std::vector<const char*> var_vector = {env_var, nullptr};
   auto env_vars = EnvironmentVariables::newWithAllowListAndEnv(allow_list, var_vector.data());
   EXPECT_THAT(
      [&]() { (void)env_vars.verify({.program_name = "some_binary_name"}); },
      ThrowsMessage<silo::config::ConfigException>(::testing::HasSubstr(
         "in environment variables: unknown variable SILO_DEBUG for 'some_binary_name'"
      ))
   );
}

TEST(EnvironmentVariables, doesNotErrorWhenThePrefixIsNotSILO_) {
   const std::vector<std::string> allow_list;
   const char* env_var = "SILODEBUG=1";
   const std::vector<const char*> var_vector = {env_var, nullptr};
   auto env_vars = EnvironmentVariables::newWithAllowListAndEnv(allow_list, var_vector.data());
   ASSERT_NO_THROW((void)env_vars.verify({}));
}

TEST(EnvironmentVariables, errorsOnWrongType) {
   const std::vector<std::string> allow_list{"SILO_FOO"};
   const char* env_var = "SILO_FOO=bar";
   const std::vector<const char*> var_vector = {env_var, nullptr};
   auto env_vars = EnvironmentVariables::newWithAllowListAndEnv(allow_list, var_vector.data());
   EXPECT_THAT(
      [&]() {
         (void)env_vars.verify(silo::config::ConfigSpecification{
            .program_name = "test",
            .attribute_specifications =
               {silo::config::ConfigAttributeSpecification::createWithoutDefault(
                  ConfigKeyPath::tryFrom({{"foo"}}).value(),
                  silo::config::ConfigValueType::INT32,
                  "some help text"
               )}
         });
      },
      ThrowsMessage<silo::config::ConfigException>(::testing::HasSubstr("cannot parse 'bar' as i32")
      )
   );
}

TEST(EnvironmentVariables, parsesVariables) {
   const std::vector<std::string> allow_list{"SILO_FOO", "SILO_FOO_INT"};
   const char* env_var1 = "SILO_FOO=bar";
   const char* env_var2 = "SILO_FOO_INT=1";
   const std::vector<const char*> var_vector = {env_var1, env_var2, nullptr};
   auto env_vars = EnvironmentVariables::newWithAllowListAndEnv(allow_list, var_vector.data());
   ASSERT_NO_THROW((void)env_vars.verify(silo::config::ConfigSpecification{
      .program_name = "test",
      .attribute_specifications =
         {silo::config::ConfigAttributeSpecification::createWithoutDefault(
             ConfigKeyPath::tryFrom({{"foo"}}).value(),
             silo::config::ConfigValueType::STRING,
             "some help text"
          ),
          silo::config::ConfigAttributeSpecification::createWithoutDefault(
             ConfigKeyPath::tryFrom({{"foo"}, {"int"}}).value(),
             silo::config::ConfigValueType::INT32,
             "some help text"
          )}
   }));
}

TEST(EnvironmentVariables, parsesVariablesWithDoubleEquals) {
   const std::vector<std::string> allow_list{"SILO_FOO"};
   const char* env_var1 = "SILO_FOO=bar=baz";
   const std::vector<const char*> var_vector = {env_var1, nullptr};
   auto env_vars = EnvironmentVariables::newWithAllowListAndEnv(allow_list, var_vector.data());
   ASSERT_EQ(
      env_vars
         .verify(silo::config::ConfigSpecification{
            .program_name = "test",
            .attribute_specifications =
               {silo::config::ConfigAttributeSpecification::createWithoutDefault(
                  ConfigKeyPath::tryFrom({{"foo"}}).value(),
                  silo::config::ConfigValueType::STRING,
                  "some help text"
               )}
         })
         .getString(ConfigKeyPath::tryFrom({{"foo"}}).value()),
      "bar=baz"
   );
}
