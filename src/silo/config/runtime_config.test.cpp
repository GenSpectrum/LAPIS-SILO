#include "silo/config/runtime_config.h"

#include <gtest/gtest.h>

#include "silo/config/util/yaml_file.h"

TEST(RuntimeConfig, shouldReadConfig) {
   silo::config::RuntimeConfig runtime_config;

   auto source = YamlFile::readFile("./testBaseData/test_runtime_config.yaml")
                    .verify(silo::config::RUNTIME_CONFIG_METADATA.configValues());

   runtime_config.overwriteFrom(ConsList<std::string>(), *source);

   ASSERT_EQ(runtime_config.api_options.data_directory, std::filesystem::path("test/directory"));
}
