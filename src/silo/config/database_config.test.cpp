#include "silo/config/database_config.h"

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include "config/config_exception.h"

using silo::config::ConfigException;
using silo::config::DatabaseConfig;
using silo::config::toDatabaseValueType;
using silo::config::ValueType;
using silo::schema::ColumnType;

TEST(DatabaseMetadataType, shouldBeConvertableFromString) {
   ASSERT_TRUE(toDatabaseValueType("string") == ValueType::STRING);
   ASSERT_TRUE(toDatabaseValueType("date") == ValueType::DATE);
   ASSERT_THROW(toDatabaseValueType("unknown"), ConfigException);
}

TEST(DatabaseConfig, shouldBuildDatabaseConfig) {
   const DatabaseConfig config = silo::config::DatabaseConfig::getValidatedConfig(
      R"(
defaultNucleotideSequence: "main"
schema:
  instanceName: "testInstanceName"
  metadata:
    - name: "metadata1"
      type: "string"
    - name: "metadata2"
      type: "string"
    - name: "testPrimaryKey"
      type: "date"
  primaryKey: "testPrimaryKey"
)"
   );
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
   bool phylo_tree_node_identifier;
   ColumnType expected_column_type;
};

class DatabaseMetadataFixture : public ::testing::TestWithParam<TestParameter> {};

TEST_P(DatabaseMetadataFixture, getColumnTypeShouldReturnCorrectColumnType) {
   const auto test_parameter = GetParam();

   const silo::config::DatabaseMetadata under_test = {
      .name = "testName",
      .type = test_parameter.value_type,
      .generate_index = test_parameter.generate_index,
      .phylo_tree_node_identifier = test_parameter.phylo_tree_node_identifier,
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
         .phylo_tree_node_identifier = true,
         .expected_column_type = ColumnType::STRING,
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
         .expected_column_type = ColumnType::INT32
      },
      TestParameter{
         .value_type = ValueType::FLOAT,
         .generate_index = false,
         .expected_column_type = ColumnType::FLOAT
      }
   )
);

TEST(DatabaseConfig, shouldReadConfigWithCorrectParameters) {
   DatabaseConfig config = silo::config::DatabaseConfig::getValidatedConfigFromFile(
      "testBaseData/test_database_config.yaml"
   );

   ASSERT_EQ(config.schema.instance_name, "sars_cov-2_minimal_test_config");
   ASSERT_EQ(config.schema.primary_key, "primary_key");
   ASSERT_EQ(config.schema.metadata.size(), 9);
   ASSERT_EQ(config.schema.metadata[0].name, "primary_key");
   ASSERT_EQ(config.schema.metadata[0].type, ValueType::STRING);
   ASSERT_EQ(config.schema.metadata[0].generate_index, false);
   ASSERT_EQ(config.schema.metadata[0].generate_lineage_index, std::nullopt);
   ASSERT_EQ(config.schema.metadata[0].phylo_tree_node_identifier, true);
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
   ASSERT_EQ(config.schema.metadata[5].generate_lineage_index, "some_test_value");
   ASSERT_EQ(config.schema.metadata[6].name, "division");
   ASSERT_EQ(config.schema.metadata[6].type, ValueType::STRING);
   ASSERT_EQ(config.schema.metadata[6].generate_index, true);
   ASSERT_EQ(config.schema.metadata[6].generate_lineage_index, std::nullopt);
   ASSERT_EQ(config.schema.metadata[7].name, "age");
   ASSERT_EQ(config.schema.metadata[7].type, ValueType::INT);
   ASSERT_EQ(config.schema.metadata[7].generate_index, false);
   ASSERT_EQ(config.schema.metadata[8].name, "qc_value");
   ASSERT_EQ(config.schema.metadata[8].type, ValueType::FLOAT);
   ASSERT_EQ(config.schema.metadata[8].generate_index, false);
   ASSERT_EQ(config.default_nucleotide_sequence, "main");
   ASSERT_EQ(config.default_amino_acid_sequence, std::nullopt);
}

TEST(DatabaseConfig, shouldThrowExceptionWhenConfigFileDoesNotExist) {
   EXPECT_THAT(
      []() {
         (void)silo::config::DatabaseConfig::getValidatedConfigFromFile(
            "testBaseData/does_not_exist.yaml"
         );
      },
      ThrowsMessage<std::runtime_error>(::testing::HasSubstr("Failed to read database config"))
   );
}

TEST(DatabaseConfig, shouldThrowErrorForInvalidMetadataType) {
   const auto* yaml = R"-(
schema:
  instanceName: dummy name
  metadata:
    - name: wrongType
      type: wrong_type
  primaryKey: primary_key
)-";

   ASSERT_THROW((void)silo::config::DatabaseConfig::getValidatedConfig(yaml), ConfigException);
}

TEST(DatabaseConfig, shouldNotThrowIfThereAreAdditionalEntries) {
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

   ASSERT_NO_THROW((void)silo::config::DatabaseConfig::getValidatedConfig(yaml));
}

TEST(DatabaseConfig, shouldThrowIfTheConfigHasAnInvalidStructure) {
   const auto* yaml = R"-(
schema:
  instanceName: dummy name
  primaryKey: missing metadata
)-";

   EXPECT_THAT(
      [yaml]() { (void)silo::config::DatabaseConfig::getValidatedConfig(yaml); },
      ThrowsMessage<std::runtime_error>(
         ::testing::HasSubstr("invalid node; first invalid key: \"metadata\"")
      )
   );
}

TEST(DatabaseConfig, shouldReadConfigWithDefaultSequencesSet) {
   const auto* yaml = R"-(
schema:
  instanceName: dummy with default
  metadata:
    - name: primaryKey
      type: string
  primaryKey: primaryKey
defaultNucleotideSequence: defaultNuc
defaultAminoAcidSequence: defaultAA
)-";

   const DatabaseConfig& config = silo::config::DatabaseConfig::getValidatedConfig(yaml);

   ASSERT_EQ(config.default_nucleotide_sequence, "defaultNuc");
   ASSERT_EQ(config.default_amino_acid_sequence, "defaultAA");
}

TEST(DatabaseConfig, shouldReadConfigWithDefaultSequencesSetButNull) {
   const auto* yaml = R"-(
schema:
  instanceName: dummy with no default explicitly
  metadata:
    - name: primaryKey
      type: string
  primaryKey: primaryKey
defaultNucleotideSequence: null
defaultAminoAcidSequence: null
)-";

   const DatabaseConfig& config = silo::config::DatabaseConfig::getValidatedConfig(yaml);

   ASSERT_EQ(config.default_nucleotide_sequence, std::nullopt);
   ASSERT_EQ(config.default_amino_acid_sequence, std::nullopt);
}

TEST(DatabaseConfig, shouldReadConfigWithoutErrors) {
   const char* const config_yaml =
      R"(
defaultNucleotideSequence: "main"
schema:
  instanceName: "testInstanceName"
  metadata:
    - name: "testPrimaryKey"
      type: "string"
    - name: "metadata1"
      type: "string"
      generateIndex: true
      generateLineageIndex: lineage
    - name: "metadata2"
      type: "date"
    - name: "metadata3"
      type: "string"
      isPhyloTreeField: true
  primaryKey: "testPrimaryKey"
)";

   ASSERT_NO_THROW(DatabaseConfig::getValidatedConfig(config_yaml));
}

TEST(DatabaseConfig, shouldThrowIfPrimaryKeyIsNotInMetadata) {
   const char* const config_yaml =
      R"(
defaultNucleotideSequence: "main"
schema:
  instanceName: "testInstanceName"
  metadata:
    - name: "notPrimaryKey"
      type: "string"
  primaryKey: "testPrimaryKey"
)";

   ASSERT_THROW(DatabaseConfig::getValidatedConfig(config_yaml), ConfigException);
}

TEST(DatabaseConfig, shouldThrowIfThereAreTwoMetadataWithTheSameName) {
   const char* const config_yaml =
      R"(
defaultNucleotideSequence: "main"
schema:
  instanceName: "testInstanceName"
  metadata:
    - name: "testPrimaryKey"
      type: "string"
    - name: "sameName"
      type: "string"
    - name: "sameName"
      type: "date"
  primaryKey: "testPrimaryKey"
)";

   ASSERT_THROW(DatabaseConfig::getValidatedConfig(config_yaml), ConfigException);
}

TEST(DatabaseConfig, givenMetadataToGenerateIndexForThatIsNotStringThenThrows) {
   const char* const config_yaml =
      R"(
defaultNucleotideSequence: "main"
schema:
  instanceName: "testInstanceName"
  metadata:
    - name: "testPrimaryKey"
      type: "string"
    - name: "indexed date"
      type: "date"
      generateIndex: true
  primaryKey: "testPrimaryKey"
)";

   EXPECT_THAT(
      [&config_yaml]() { DatabaseConfig::getValidatedConfig(config_yaml); },
      ThrowsMessage<ConfigException>(
         ::testing::HasSubstr("Metadata 'indexed date' generateIndex is set, but generating an "
                              "index is only allowed for types STRING")
      )
   );
}

TEST(DatabaseConfig, givenLineageIndexAndNotGenerateThenThrows) {
   const char* const config_yaml =
      R"(
defaultNucleotideSequence: "main"
schema:
  instanceName: "testInstanceName"
  metadata:
    - name: "testPrimaryKey"
      type: "string"
    - name: "some lineage"
      type: "string"
      generateLineageIndex: lineage
  primaryKey: "testPrimaryKey"
)";

   EXPECT_THAT(
      [&config_yaml]() { DatabaseConfig::getValidatedConfig(config_yaml); },
      ThrowsMessage<ConfigException>(
         ::testing::HasSubstr("Metadata 'some lineage' generateLineageIndex is set, "
                              "generateIndex must also be set")
      )
   );
}

TEST(DatabaseConfig, givenPhyloTreeIndexAndGenerateThenThrows) {
   const char* const config_yaml =
      R"(
defaultNucleotideSequence: "main"
schema:
  instanceName: "testInstanceName"
  metadata:
    - name: "testPrimaryKey"
      type: "string"
    - name: "some lineage"
      type: "string"
      isPhyloTreeField: true
      generateIndex: true
  primaryKey: "testPrimaryKey"
)";

   EXPECT_THAT(
      [&config_yaml]() { DatabaseConfig::getValidatedConfig(config_yaml); },
      ThrowsMessage<ConfigException>(
         ::testing::HasSubstr("Metadata 'some lineage' isPhyloTreeField and generateIndex "
                              "are both set, if isPhyloTreeField is "
                              "set then generateIndex cannot be set.")
      )
   );
}

}  // namespace
