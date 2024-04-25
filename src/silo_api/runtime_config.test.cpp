#include "silo_api/runtime_config.h"

#include <gtest/gtest.h>

TEST(RuntimeConfig, shouldReadConfig) {
   silo_api::RuntimeConfig runtime_config;
   runtime_config.overwriteFromFile("./testBaseData/test_runtime_config.yaml");

   ASSERT_EQ(runtime_config.data_directory, std::filesystem::path("test/directory"));
}
