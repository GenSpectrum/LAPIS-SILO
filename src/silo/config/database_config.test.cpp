#include "silo/config/database_config.h"

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include "silo/config/util/config_exception.h"

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
   ASSERT_THROW(toDatabaseValueType("unknown"), ConfigException);
}

TEST(DatabaseConfig, shouldBuildDatabaseConfig) {
   const std::string default_nuc_sequence = "main";
   const DatabaseSchema schema{
      .instance_name = "testInstanceName",
      .metadata =
         {
            {.name = "metadata1", .type = ValueType::STRING},
            {.name = "metadata2", .type = ValueType::STRING},
            {.name = "metadata3", .type = ValueType::DATE},
         },
      .primary_key = "testPrimaryKey",
   };
   const DatabaseConfig config{
      .default_nucleotide_sequence = default_nuc_sequence, .schema = schema
   };
   ASSERT_TRUE(config.schema.instance_name == "testInstanceName");
   ASSERT_TRUE(config.schema.primary_key == "testPrimaryKey");
   ASSERT_TRUE(config.schema.metadata[0].name == "metadata1");
   ASSERT_TRUE(config.schema.metadata.size() == 3);
}

namespace {

struct TestParameter {
   ValueType value_type;
   bool generate_index;
   bool generate_lineage_index;
   ColumnType expected_column_type;
};

class DatabaseMetadataFixture : public ::testing::TestWithParam<TestParameter> {};

TEST_P(DatabaseMetadataFixture, getColumnTypeShouldReturnCorrectColumnType) {
   const auto test_parameter = GetParam();

   const silo::config::DatabaseMetadata under_test = {
      .name = "testName",
      .type = test_parameter.value_type,
      .generate_index = test_parameter.generate_index,
   };

   ASSERT_EQ(under_test.getColumnType(), test_parameter.expected_column_type);
}

INSTANTIATE_TEST_SUITE_P(
   DatabaseMetadata,
   DatabaseMetadataFixture,
   ::testing::Values(
      TestParameter{
         .value_type = ValueType::STRING,
         .generate_index = false,
         .expected_column_type = ColumnType::STRING
      },
      TestParameter{
         .value_type = ValueType::STRING,
         .generate_index = true,
         .expected_column_type = ColumnType::INDEXED_STRING
      },
      TestParameter{
         .value_type = ValueType::DATE,
         .generate_index = false,
         .expected_column_type = ColumnType::DATE
      },
      TestParameter{
         .value_type = ValueType::STRING,
         .generate_index = true,
         .generate_lineage_index = true,
         .expected_column_type = ColumnType::INDEXED_STRING
      },
      TestParameter{
         .value_type = ValueType::INT,
         .generate_index = false,
         .expected_column_type = ColumnType::INT
      },
      TestParameter{
         .value_type = ValueType::FLOAT,
         .generate_index = false,
         .expected_column_type = ColumnType::FLOAT
      }
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
   ASSERT_EQ(config.schema.metadata[5].type, ValueType::STRING);
   ASSERT_EQ(config.schema.metadata[5].generate_index, true);
   ASSERT_EQ(config.schema.metadata[5].generate_lineage_index, true);
   ASSERT_EQ(config.schema.metadata[6].name, "division");
   ASSERT_EQ(config.schema.metadata[6].type, ValueType::STRING);
   ASSERT_EQ(config.schema.metadata[6].generate_index, true);
   ASSERT_EQ(config.schema.metadata[6].generate_lineage_index, false);
   ASSERT_EQ(config.schema.metadata[7].name, "age");
   ASSERT_EQ(config.schema.metadata[7].type, ValueType::INT);
   ASSERT_EQ(config.schema.metadata[7].generate_index, false);
   ASSERT_EQ(config.schema.metadata[8].name, "qc_value");
   ASSERT_EQ(config.schema.metadata[8].type, ValueType::FLOAT);
   ASSERT_EQ(config.schema.metadata[8].generate_index, false);
   ASSERT_EQ(config.default_nucleotide_sequence, std::nullopt);
   ASSERT_EQ(config.default_amino_acid_sequence, std::nullopt);
}

TEST(DatabaseConfigReader, shouldThrowExceptionWhenConfigFileDoesNotExist) {
   EXPECT_THAT(
      []() { (void)DatabaseConfigReader().readConfig("testBaseData/does_not_exist.yaml"); },
      ThrowsMessage<std::runtime_error>(::testing::HasSubstr("Failed to read database config"))
   );
}

TEST(DatabaseConfigReader, shouldThrowErrorForInvalidMetadataType) {
   const auto* yaml = R"-(
schema:
  instanceName: dummy name
  metadata:
    - name: wrongType
      type: wrong_type
  primaryKey: gisaid_epi_isl
)-";

   ASSERT_THROW((void)DatabaseConfigReader().parseYaml(yaml), ConfigException);
}

TEST(DatabaseConfigReader, shouldNotThrowIfThereAreAdditionalEntries) {
   const auto* yaml = R"-(
schema:
  instanceName: dummy name
  metadata:
    - name: key
      type: string
  primaryKey: key
  features:
    - name: this is unknown to SILO
)-";

   ASSERT_NO_THROW((void)DatabaseConfigReader().parseYaml(yaml));
}

TEST(DatabaseConfigReader, shouldThrowIfTheConfigHasAnInvalidStructure) {
   const auto* yaml = R"-(
schema:
  instanceName: dummy name
  primaryKey: missing metadata
)-";

   EXPECT_THAT(
      [yaml]() { (void)DatabaseConfigReader().parseYaml(yaml); },
      ThrowsMessage<std::runtime_error>(
         ::testing::HasSubstr("invalid node; first invalid key: \"metadata\"")
      )
   );
}

TEST(DatabaseConfigReader, shouldReadConfigWithoutDateToSortBy) {
   const auto* yaml = R"-(
schema:
  instanceName: Having no dateToSortBy is valid
  metadata:
    - name: primaryKey
      type: string
  primaryKey: primaryKey
  partitionBy: pango_lineage
)-";

   const DatabaseConfig& config = DatabaseConfigReader().parseYaml(yaml);

   ASSERT_EQ(config.schema.date_to_sort_by, std::nullopt);
}

TEST(DatabaseConfigReader, shouldReadConfigWithoutPartitionBy) {
   const auto* yaml = R"-(
schema:
  instanceName: dummy without partitionBy
  metadata:
    - name: primaryKey
      type: string
    - name: date
      type: date
  primaryKey: primaryKey
  dateToSortBy: date
)-";

   const DatabaseConfig& config = DatabaseConfigReader().parseYaml(yaml);

   ASSERT_EQ(config.schema.partition_by, std::nullopt);
}

TEST(DatabaseConfigReader, shouldReadConfigWithDefaultSequencesSet) {
   const auto* yaml = R"-(
schema:
  instanceName: dummy without partitionBy
  metadata:
    - name: primaryKey
      type: string
  primaryKey: primaryKey
defaultNucleotideSequence: defaultNuc
defaultAminoAcidSequence: defaultAA
)-";

   const DatabaseConfig& config = DatabaseConfigReader().parseYaml(yaml);

   ASSERT_EQ(config.default_nucleotide_sequence, "defaultNuc");
   ASSERT_EQ(config.default_amino_acid_sequence, "defaultAA");
}

TEST(DatabaseConfigReader, shouldReadConfigWithDefaultSequencesSetButNull) {
   const auto* yaml = R"-(
schema:
  instanceName: dummy without partitionBy
  metadata:
    - name: primaryKey
      type: string
  primaryKey: primaryKey
defaultNucleotideSequence: null
defaultAminoAcidSequence: null
)-";

   const DatabaseConfig& config = DatabaseConfigReader().parseYaml(yaml);

   ASSERT_EQ(config.default_nucleotide_sequence, std::nullopt);
   ASSERT_EQ(config.default_amino_acid_sequence, std::nullopt);
}

}  // namespace
