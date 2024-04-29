#include "silo/config/runtime_config.h"

#include <gtest/gtest.h>

#include "silo/config/util/yaml_config.h"

TEST(RuntimeConfig, shouldReadConfig) {
   silo_api::RuntimeConfig runtime_config;
   runtime_config.overwrite(silo::config::YamlConfig("./testBaseData/test_runtime_config.yaml"));

   ASSERT_EQ(runtime_config.data_directory, std::filesystem::path("test/directory"));
}
