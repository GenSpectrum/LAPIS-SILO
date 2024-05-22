#include "silo/config/runtime_config.h"

#include <gtest/gtest.h>

#include "silo/config/util/yaml_file.h"

TEST(RuntimeConfig, shouldReadConfig) {
   silo::config::RuntimeConfig runtime_config;
   runtime_config.overwrite(silo::config::YamlFile("./testBaseData/test_runtime_config.yaml"));

   ASSERT_EQ(runtime_config.api_options.data_directory, std::filesystem::path("test/directory"));
}
