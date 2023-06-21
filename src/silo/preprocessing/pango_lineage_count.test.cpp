#include "silo/storage/pango_lineage_alias.h"

#include <gtest/gtest.h>
#include <filesystem>

#include <silo/config/database_config.h>
#include "silo/preprocessing/pango_lineage_count.h"

TEST(PangoLineageCounts, buildPangoLineageCounts) {
   const std::filesystem::path metadata_path = "testBaseData/small_metadata_set.tsv";
   const silo::config::DatabaseConfig database_config = {
      .schema = {.partition_by = "pango_lineage"}};

   auto result = silo::preprocessing::buildPangoLineageCounts(
      silo::PangoLineageAliasLookup::readFromFile("testBaseData/pango_alias.txt"),
      metadata_path,
      database_config
   );

   ASSERT_EQ(result.pango_lineage_counts.size(), 24);
   ASSERT_EQ(result.pango_lineage_counts[0].pango_lineage, "B.1");
   ASSERT_EQ(result.pango_lineage_counts[0].count_of_sequences, 3);
   ASSERT_EQ(result.pango_lineage_counts[6].pango_lineage, "B.1.1.7");
   ASSERT_EQ(result.pango_lineage_counts[6].count_of_sequences, 48);
   ASSERT_EQ(result.pango_lineage_counts[23].pango_lineage, "B.1.617.2.9.2");
   ASSERT_EQ(result.pango_lineage_counts[23].count_of_sequences, 1);
}
