#include "silo/config/database_config_reader.h"
#include "silo/config/config_exception.h"

#include <gtest/gtest.h>

TEST(DatabaseConfigReader, shouldReadConfigWithoutErrors) {
   ASSERT_NO_THROW(silo::DatabaseConfigReader().readConfig("testBaseData/test_database_config.yaml")
   );
}

TEST(DatabaseConfigReader, shouldReadConfigWithCorrectParameters) {
   const auto config =
      silo::DatabaseConfigReader().readConfig("testBaseData/test_database_config.yaml");

   ASSERT_EQ(config.schema.instance_name, "sars_cov-2_minimal_test_config");
   ASSERT_EQ(config.schema.primary_key, "gisaid_epi_isl");
   ASSERT_EQ(config.schema.metadata.size(), 5);
   ASSERT_EQ(config.schema.metadata[0].name, "gisaid_epi_isl");
   ASSERT_EQ(config.schema.metadata[0].type, silo::DatabaseMetadataType::STRING);
   ASSERT_EQ(config.schema.metadata[1].name, "date");
   ASSERT_EQ(config.schema.metadata[1].type, silo::DatabaseMetadataType::DATE);
   ASSERT_EQ(config.schema.metadata[2].name, "region");
   ASSERT_EQ(config.schema.metadata[2].type, silo::DatabaseMetadataType::STRING);
   ASSERT_EQ(config.schema.metadata[3].name, "country");
   ASSERT_EQ(config.schema.metadata[3].type, silo::DatabaseMetadataType::STRING);
   ASSERT_EQ(config.schema.metadata[4].name, "pangoLineage");
   ASSERT_EQ(config.schema.metadata[4].type, silo::DatabaseMetadataType::PANGOLINEAGE);
}

TEST(DatabaseConfigReader, shouldThrowExceptionWhenConfigFileDoesNotExist) {
   ASSERT_THROW(
      silo::DatabaseConfigReader().readConfig("testBaseData/does_not_exist.yaml"),
      std::runtime_error
   );
}

TEST(DatabaseConfigReader, shouldThrowErrorForInvalidMetadataType) {
   ASSERT_THROW(
      silo::DatabaseConfigReader().readConfig(
         "testBaseData/test_database_config_with_invalid_metadata_type.yaml"
      ),
      silo::ConfigException
   );
}

TEST(DataBaseConfigReader, shouldNotThrowIfThereAreAdditionalEntries) {
   ASSERT_NO_THROW(silo::DatabaseConfigReader().readConfig(
      "testBaseData/test_database_config_with_additional_entries.yaml"
   ));
}
