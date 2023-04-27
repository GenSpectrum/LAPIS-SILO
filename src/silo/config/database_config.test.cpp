#include "silo/config/database_config.h"

#include <gtest/gtest.h>

TEST(DatabaseConfig, shouldBuildDatabaseConfig) {
   const silo::DatabaseSchema schema{
      "testInstanceName",
      {
         {"metadata1", silo::DatabaseMetadataType::PANGOLINEAGE},
         {"metadata2", silo::DatabaseMetadataType::STRING},
         {"metadata3", silo::DatabaseMetadataType::DATE},
      },
      "testPrimaryKey",
   };
   const silo::DatabaseConfig config{schema};
   ASSERT_TRUE(config.schema.instance_name == "testInstanceName");
   ASSERT_TRUE(config.schema.primary_key == "testPrimaryKey");
   ASSERT_TRUE(config.schema.metadata[0].name == "metadata1");
   ASSERT_TRUE(config.schema.metadata.size() == 3);
}