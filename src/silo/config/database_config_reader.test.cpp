#include "silo/config/database_config_reader.h"
#include "silo/config/config_exception.h"

#include <gtest/gtest.h>

#include <yaml-cpp/yaml.h>

using silo::config::ConfigException;
using silo::config::DatabaseConfig;
using silo::config::DatabaseConfigReader;
using silo::config::DatabaseMetadataType;

TEST(DatabaseConfigReader, shouldReadConfigWithCorrectParameters) {
   DatabaseConfig config;
   ASSERT_NO_THROW(
      config = DatabaseConfigReader().readConfig("testBaseData/test_database_config.yaml")
   );

   ASSERT_EQ(config.schema.instance_name, "sars_cov-2_minimal_test_config");
   ASSERT_EQ(config.schema.primary_key, "gisaid_epi_isl");
   ASSERT_EQ(config.schema.metadata.size(), 6);
   ASSERT_EQ(config.schema.metadata[0].name, "gisaid_epi_isl");
   ASSERT_EQ(config.schema.metadata[0].type, DatabaseMetadataType::STRING);
   ASSERT_EQ(config.schema.metadata[1].name, "date");
   ASSERT_EQ(config.schema.metadata[1].type, DatabaseMetadataType::DATE);
   ASSERT_EQ(config.schema.metadata[2].name, "region");
   ASSERT_EQ(config.schema.metadata[2].type, DatabaseMetadataType::STRING);
   ASSERT_EQ(config.schema.metadata[3].name, "country");
   ASSERT_EQ(config.schema.metadata[3].type, DatabaseMetadataType::STRING);
   ASSERT_EQ(config.schema.metadata[4].name, "pango_lineage");
   ASSERT_EQ(config.schema.metadata[4].type, DatabaseMetadataType::PANGOLINEAGE);
   ASSERT_EQ(config.schema.metadata[5].name, "division");
   ASSERT_EQ(config.schema.metadata[5].type, DatabaseMetadataType::STRING);
}

TEST(DatabaseConfigReader, shouldThrowExceptionWhenConfigFileDoesNotExist) {
   ASSERT_THROW(
      DatabaseConfigReader().readConfig("testBaseData/does_not_exist.yaml"), std::runtime_error
   );
}

TEST(DatabaseConfigReader, shouldThrowErrorForInvalidMetadataType) {
   ASSERT_THROW(
      DatabaseConfigReader().readConfig(
         "testBaseData/test_database_config_with_invalid_metadata_type.yaml"
      ),
      ConfigException
   );
}

TEST(DatabaseConfigReader, shouldNotThrowIfThereAreAdditionalEntries) {
   ASSERT_NO_THROW(DatabaseConfigReader().readConfig(
      "testBaseData/test_database_config_with_additional_entries.yaml"
   ));
}

TEST(DatabaseConfigReader, shouldThrowIfTheConfigHasAnInvalidStructure) {
   ASSERT_THROW(
      DatabaseConfigReader().readConfig(
         "testBaseData/test_database_config_with_invalid_structure.yaml"
      ),
      YAML::InvalidNode
   );
}
