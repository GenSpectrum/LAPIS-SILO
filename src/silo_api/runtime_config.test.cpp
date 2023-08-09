#include "silo_api/runtime_config.h"

#include <gtest/gtest.h>

TEST(RuntimeConfig, shouldReadConfig) {
   const auto result =
      silo_api::RuntimeConfig::readFromFile("./testBaseData/test_runtime_config.yaml");

   ASSERT_EQ(result.data_directory, std::filesystem::path("test/directory"));
}
