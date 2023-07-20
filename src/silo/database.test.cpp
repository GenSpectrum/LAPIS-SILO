#include "silo/database.h"

#include <gtest/gtest.h>

#include "silo/common/nucleotide_symbols.h"
#include "silo/config/config_repository.h"
#include "silo/database_info.h"
#include "silo/preprocessing/preprocessing_config.h"
#include "silo/preprocessing/preprocessing_config_reader.h"
#include "silo/storage/database_partition.h"

silo::Database buildTestDatabase() {
   const silo::preprocessing::InputDirectory input_directory{"./testBaseData/"};

   auto config = silo::preprocessing::PreprocessingConfigReader().readConfig(
      input_directory.directory + "test_preprocessing_config.yaml"
   );

   const auto database_config = silo::config::ConfigRepository().getValidatedConfig(
      input_directory.directory + "test_database_config.yaml"
   );

   return silo::Database::preprocessing(config, database_config);
}

TEST(DatabaseTest, shouldBuildDatabaseWithoutErrors) {
   auto database{buildTestDatabase()};

   const auto simple_database_info = database.getDatabaseInfo();

   EXPECT_GT(simple_database_info.total_size, 0);
   EXPECT_EQ(simple_database_info.sequence_count, 100);
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST(DatabaseTest, shouldReturnCorrectDatabaseInfo) {
   auto database{buildTestDatabase()};

   const auto detailed_info = database.detailedDatabaseInfo().sequences.at("main");
   const auto simple_info = database.getDatabaseInfo();

   EXPECT_EQ(
      detailed_info.bitmap_size_per_symbol.size_in_bytes.at(silo::NUCLEOTIDE_SYMBOL::A), 9190510
   );
   EXPECT_EQ(
      detailed_info.bitmap_size_per_symbol.size_in_bytes.at(silo::NUCLEOTIDE_SYMBOL::GAP), 5779958
   );

   EXPECT_EQ(
      detailed_info.bitmap_container_size_per_genome_section.bitmap_container_size_statistic
         .number_of_bitset_containers,
      0
   );
   EXPECT_EQ(
      detailed_info.bitmap_container_size_per_genome_section.bitmap_container_size_statistic
         .number_of_values_stored_in_run_containers,
      0
   );
   EXPECT_EQ(
      detailed_info.bitmap_container_size_per_genome_section.bitmap_container_size_statistic
         .total_bitmap_size_bitset_containers,
      0
   );

   EXPECT_EQ(
      detailed_info.bitmap_container_size_per_genome_section.total_bitmap_size_computed, 103449226
   );
   EXPECT_EQ(
      detailed_info.bitmap_container_size_per_genome_section.total_bitmap_size_frozen, 55370197
   );
   EXPECT_EQ(
      detailed_info.bitmap_container_size_per_genome_section.bitmap_container_size_statistic
         .total_bitmap_size_array_containers,
      5859154
   );

   EXPECT_EQ(simple_info.total_size, 66467326);
   EXPECT_EQ(simple_info.sequence_count, 100);
   EXPECT_EQ(simple_info.n_bitmaps_size, 3898);
}
