#include "silo/config/database_config.h"

#include <gtest/gtest.h>

TEST(DatabaseConfig, shouldBuildDatabaseConfigWithoutErrors) {
   const silo::DatabaseSchema schema{
      "test",
      {
         {"name", "string"},
         {"type", "string"},
      },
      "name",
   };
   const silo::DatabaseConfig config{schema};
   assert(config.schema.instance_name == "test");
}