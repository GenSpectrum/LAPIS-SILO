#include "silo/config/database_config_reader.h"
#include "silo/config/config_exception.h"

#include <gtest/gtest.h>

#include <yaml-cpp/yaml.h>

using silo::config::ConfigException;
using silo::config::DatabaseConfig;
using silo::config::DatabaseConfigReader;
using silo::config::ValueType;

TEST(DatabaseConfigReader, shouldReadConfigWithCorrectParameters) {
   DatabaseConfig config;
   ASSERT_NO_THROW(
      config = DatabaseConfigReader().readConfig("testBaseData/test_database_config.yaml")
   );

   ASSERT_EQ(config.schema.instance_name, "sars_cov-2_minimal_test_config");
   ASSERT_EQ(config.schema.primary_key, "gisaid_epi_isl");
   ASSERT_EQ(config.schema.date_to_sort_by, "date");
   ASSERT_EQ(config.schema.partition_by, "pango_lineage");
   ASSERT_EQ(config.schema.metadata.size(), 9);
   ASSERT_EQ(config.schema.metadata[0].name, "gisaid_epi_isl");
   ASSERT_EQ(config.schema.metadata[0].type, ValueType::STRING);
   ASSERT_EQ(config.schema.metadata[0].generate_index, false);
   ASSERT_EQ(config.schema.metadata[1].name, "date");
   ASSERT_EQ(config.schema.metadata[1].type, ValueType::DATE);
   ASSERT_EQ(config.schema.metadata[1].generate_index, false);
   ASSERT_EQ(config.schema.metadata[2].name, "unsorted_date");
   ASSERT_EQ(config.schema.metadata[2].type, ValueType::DATE);
   ASSERT_EQ(config.schema.metadata[2].generate_index, false);
   ASSERT_EQ(config.schema.metadata[3].name, "region");
   ASSERT_EQ(config.schema.metadata[3].type, ValueType::STRING);
   ASSERT_EQ(config.schema.metadata[3].generate_index, true);
   ASSERT_EQ(config.schema.metadata[4].name, "country");
   ASSERT_EQ(config.schema.metadata[4].type, ValueType::STRING);
   ASSERT_EQ(config.schema.metadata[4].generate_index, true);
   ASSERT_EQ(config.schema.metadata[5].name, "pango_lineage");
   ASSERT_EQ(config.schema.metadata[5].type, ValueType::PANGOLINEAGE);
   ASSERT_EQ(config.schema.metadata[5].generate_index, true);
   ASSERT_EQ(config.schema.metadata[6].name, "division");
   ASSERT_EQ(config.schema.metadata[6].type, ValueType::STRING);
   ASSERT_EQ(config.schema.metadata[6].generate_index, true);
   ASSERT_EQ(config.schema.metadata[7].name, "age");
   ASSERT_EQ(config.schema.metadata[7].type, ValueType::INT);
   ASSERT_EQ(config.schema.metadata[7].generate_index, false);
   ASSERT_EQ(config.schema.metadata[8].name, "qc_value");
   ASSERT_EQ(config.schema.metadata[8].type, ValueType::FLOAT);
   ASSERT_EQ(config.schema.metadata[8].generate_index, false);
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

TEST(DatabaseConfigReader, shouldReadConfigWithoutDateToSortBy) {
   const DatabaseConfig& config = DatabaseConfigReader().readConfig(
      "testBaseData/test_database_config_without_date_to_sort_by.yaml"
   );

   ASSERT_EQ(config.schema.date_to_sort_by, std::nullopt);
}
