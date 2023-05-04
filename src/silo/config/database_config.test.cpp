#include "silo/config/database_config.h"
#include "silo/config/config_exception.h"

#include <gtest/gtest.h>

using silo::config::ConfigException;
using silo::config::DatabaseConfig;
using silo::config::DatabaseMetadataType;
using silo::config::DatabaseSchema;
using silo::config::toDatabaseMetadataType;

TEST(DatabaseMetadataType, shouldBeConvertableFromString) {
   ASSERT_TRUE(toDatabaseMetadataType("string") == DatabaseMetadataType::STRING);
   ASSERT_TRUE(toDatabaseMetadataType("date") == DatabaseMetadataType::DATE);
   ASSERT_TRUE(toDatabaseMetadataType("pango_lineage") == DatabaseMetadataType::PANGOLINEAGE);
   ASSERT_THROW(toDatabaseMetadataType("unknown"), ConfigException);
}

TEST(DatabaseConfig, shouldBuildDatabaseConfig) {
   const DatabaseSchema schema{
      "testInstanceName",
      {
         {"metadata1", DatabaseMetadataType::PANGOLINEAGE},
         {"metadata2", DatabaseMetadataType::STRING},
         {"metadata3", DatabaseMetadataType::DATE},
      },
      "testPrimaryKey",
   };
   const DatabaseConfig config{schema};
   ASSERT_TRUE(config.schema.instance_name == "testInstanceName");
   ASSERT_TRUE(config.schema.primary_key == "testPrimaryKey");
   ASSERT_TRUE(config.schema.metadata[0].name == "metadata1");
   ASSERT_TRUE(config.schema.metadata.size() == 3);
}