#include "silo/config/database_config.h"

#include <gtest/gtest.h>
#include <yaml-cpp/yaml.h>

#include "silo/config/config_exception.h"

using silo::config::ColumnType;
using silo::config::ConfigException;
using silo::config::DatabaseConfig;
using silo::config::DatabaseConfigReader;
using silo::config::DatabaseSchema;
using silo::config::toDatabaseValueType;
using silo::config::ValueType;

TEST(DatabaseMetadataType, shouldBeConvertableFromString) {
   ASSERT_TRUE(toDatabaseValueType("string") == ValueType::STRING);
   ASSERT_TRUE(toDatabaseValueType("date") == ValueType::DATE);
   ASSERT_TRUE(toDatabaseValueType("pango_lineage") == ValueType::PANGOLINEAGE);
   ASSERT_THROW(toDatabaseValueType("unknown"), ConfigException);
}

TEST(DatabaseConfig, shouldBuildDatabaseConfig) {
   const std::string default_nuc_sequence = "main";
   const DatabaseSchema schema{
      "testInstanceName",
      {
         {"metadata1", ValueType::PANGOLINEAGE},
         {"metadata2", ValueType::STRING},
         {"metadata3", ValueType::DATE},
      },
      "testPrimaryKey",
   };
   const DatabaseConfig config{default_nuc_sequence, schema};
   ASSERT_TRUE(config.schema.instance_name == "testInstanceName");
   ASSERT_TRUE(config.schema.primary_key == "testPrimaryKey");
   ASSERT_TRUE(config.schema.metadata[0].name == "metadata1");
   ASSERT_TRUE(config.schema.metadata.size() == 3);
}

namespace {

struct TestParameter {
   ValueType value_type;
   bool generate_index;
   ColumnType expected_column_type;
};

class DatabaseMetadataFixture : public ::testing::TestWithParam<TestParameter> {
  protected:
   std::string something;
};

TEST_P(DatabaseMetadataFixture, getColumnTypeShouldReturnCorrectColumnType) {
   const auto test_parameter = GetParam();

   const silo::config::DatabaseMetadata under_test = {
      "testName",
      test_parameter.value_type,
      test_parameter.generate_index,
   };

   ASSERT_EQ(under_test.getColumnType(), test_parameter.expected_column_type);
}

INSTANTIATE_TEST_SUITE_P(
   DatabaseMetadata,
   DatabaseMetadataFixture,
   ::testing::Values(
      TestParameter{ValueType::STRING, false, ColumnType::STRING},
      TestParameter{ValueType::STRING, true, ColumnType::INDEXED_STRING},
      TestParameter{ValueType::DATE, false, ColumnType::DATE},
      TestParameter{ValueType::PANGOLINEAGE, true, ColumnType::INDEXED_PANGOLINEAGE},
      TestParameter{ValueType::INT, false, ColumnType::INT},
      TestParameter{ValueType::FLOAT, false, ColumnType::FLOAT},
      TestParameter{ValueType::INSERTION, false, ColumnType::INSERTION},
      TestParameter{ValueType::AA_INSERTION, false, ColumnType::AA_INSERTION}
   )
);

TEST(DatabaseConfigReader, shouldReadConfigWithCorrectParameters) {
   DatabaseConfig config;
   ASSERT_NO_THROW(
      config = DatabaseConfigReader().readConfig("testBaseData/test_database_config.yaml")
   );

   ASSERT_EQ(config.schema.instance_name, "sars_cov-2_minimal_test_config");
   ASSERT_EQ(config.schema.primary_key, "gisaid_epi_isl");
   ASSERT_EQ(config.schema.date_to_sort_by, "date");
   ASSERT_EQ(config.schema.partition_by, "pango_lineage");
   ASSERT_EQ(config.schema.metadata.size(), 10);
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
   ASSERT_EQ(config.schema.metadata[9].name, "insertions");
   ASSERT_EQ(config.schema.metadata[9].type, ValueType::INSERTION);
   ASSERT_EQ(config.schema.metadata[9].generate_index, false);
   ASSERT_EQ(config.schema.metadata[10].name, "aaInsertions");
   ASSERT_EQ(config.schema.metadata[10].type, ValueType::AA_INSERTION);
   ASSERT_EQ(config.schema.metadata[10].generate_index, false);
}

TEST(DatabaseConfigReader, shouldThrowExceptionWhenConfigFileDoesNotExist) {
   ASSERT_THROW(
      (void)DatabaseConfigReader().readConfig("testBaseData/does_not_exist.yaml"),
      std::runtime_error
   );
}

TEST(DatabaseConfigReader, shouldThrowErrorForInvalidMetadataType) {
   ASSERT_THROW(
      (void)DatabaseConfigReader().readConfig(
         "testBaseData/test_database_config_with_invalid_metadata_type.yaml"
      ),
      ConfigException
   );
}

TEST(DatabaseConfigReader, shouldNotThrowIfThereAreAdditionalEntries) {
   ASSERT_NO_THROW((void)DatabaseConfigReader().readConfig(
      "testBaseData/test_database_config_with_additional_entries.yaml"
   ));
}

TEST(DatabaseConfigReader, shouldThrowIfTheConfigHasAnInvalidStructure) {
   ASSERT_THROW(
      (void)DatabaseConfigReader().readConfig(
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

TEST(DatabaseConfigReader, shouldReadConfigWithoutPartitionBy) {
   const DatabaseConfig& config = DatabaseConfigReader().readConfig(
      "testBaseData/test_database_config_without_partition_by.yaml"
   );

   ASSERT_EQ(config.schema.partition_by, std::nullopt);
}

}  // namespace
