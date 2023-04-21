#include "silo/config/database_config_reader.h"

#include <gtest/gtest.h>

TEST(DatabaseConfigReader, shouldReadConfigWithoutErrors) {
   const auto config =
      silo::DatabaseConfigReader::readConfig("testBaseData/test_database_config.yaml");

   ASSERT_EQ(config.schema.instance_name, "sars_cov-2_minimal_test_config");
   ASSERT_EQ(config.schema.primary_key, "gisaid_epi_isl");
   ASSERT_EQ(config.schema.metadata.size(), 5);
   ASSERT_EQ(config.schema.metadata[0].name, "gisaid_epi_isl");
   ASSERT_EQ(config.schema.metadata[0].type, "string");
   ASSERT_EQ(config.schema.metadata[1].name, "date");
   ASSERT_EQ(config.schema.metadata[1].type, "date");
   ASSERT_EQ(config.schema.metadata[2].name, "region");
   ASSERT_EQ(config.schema.metadata[2].type, "string");
   ASSERT_EQ(config.schema.metadata[3].name, "country");
   ASSERT_EQ(config.schema.metadata[3].type, "string");
   ASSERT_EQ(config.schema.metadata[4].name, "pangoLineage");
   ASSERT_EQ(config.schema.metadata[4].type, "pango_lineage");
}