#include "silo/config/database_config.h"

#include <gtest/gtest.h>

#include "silo/config/config_exception.h"

using silo::config::ColumnType;
using silo::config::ConfigException;
using silo::config::DatabaseConfig;
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
   const DatabaseSchema schema{
      "testInstanceName",
      {
         {"metadata1", ValueType::PANGOLINEAGE},
         {"metadata2", ValueType::STRING},
         {"metadata3", ValueType::DATE},
      },
      "testPrimaryKey",
   };
   const DatabaseConfig config{schema};
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
      TestParameter{ValueType::FLOAT, false, ColumnType::FLOAT}
   )
);

}  // namespace
