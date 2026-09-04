#include "rhydb/config/runtime_config.h"

#include <gtest/gtest.h>

#include "config/source/yaml_file.h"

using rhydb::config::RuntimeConfig;
using rhydb::config::YamlFile;

TEST(RuntimeConfig, shouldReadConfig) {
   auto runtime_config = RuntimeConfig::withDefaults();

   auto source = YamlFile::readFile("./testBaseData/test_runtime_config.yaml")
                    .verify(RuntimeConfig::getConfigSpecification());

   runtime_config.overwriteFrom(source);

   ASSERT_EQ(runtime_config.api_options.address, "127.0.0.1");
   ASSERT_EQ(runtime_config.api_options.port, 1234);
   ASSERT_EQ(runtime_config.data_directory, "test/directory");
}
