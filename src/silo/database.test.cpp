#include "silo/database.h"

#include "filesystem"

#include <gtest/gtest.h>

#include "silo/common/nucleotide_symbols.h"
#include "silo/config/config_repository.h"
#include "silo/database_info.h"
#include "silo/preprocessing/preprocessing_config.h"
#include "silo/preprocessing/preprocessing_config_reader.h"

silo::Database buildTestDatabase() {
   const silo::preprocessing::InputDirectory input_directory{"./testBaseData/exampleDataset/"};

   auto config = silo::preprocessing::PreprocessingConfigReader()
                    .readConfig("./testBaseData/test_preprocessing_config.yaml")
                    .mergeValuesFromOrDefault(silo::preprocessing::OptionalPreprocessingConfig());

   const auto database_config = silo::config::ConfigRepository().getValidatedConfig(
      input_directory.directory + "database_config.yaml"
   );

   return silo::Database::preprocessing(config, database_config);
}

TEST(DatabaseTest, shouldBuildDatabaseWithoutErrors) {
   auto database{buildTestDatabase()};

   const auto simple_database_info = database.getDatabaseInfo();

   EXPECT_GT(simple_database_info.total_size, 0);
   EXPECT_EQ(simple_database_info.sequence_count, 100);
}

TEST(DatabaseTest, shouldSuccessfullyBuildDatabaseWithoutPartitionBy) {
   const silo::preprocessing::InputDirectory input_directory{"./testBaseData/"};

   auto config = silo::preprocessing::PreprocessingConfigReader()
                    .readConfig(input_directory.directory + "test_preprocessing_config.yaml")
                    .mergeValuesFromOrDefault(silo::preprocessing::OptionalPreprocessingConfig());

   const auto database_config = silo::config::ConfigRepository().getValidatedConfig(
      input_directory.directory + "test_database_config_without_partition_by.yaml"
   );

   auto database = silo::Database::preprocessing(config, database_config);

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
      detailed_info.bitmap_size_per_symbol.size_in_bytes.at(silo::NUCLEOTIDE_SYMBOL::A), 6112653
   );
   EXPECT_EQ(
      detailed_info.bitmap_size_per_symbol.size_in_bytes.at(silo::NUCLEOTIDE_SYMBOL::GAP), 6003470
   );

   EXPECT_EQ(
      detailed_info.bitmap_container_size_per_genome_section.bitmap_container_size_statistic
         .number_of_bitset_containers,
      0
   );
   EXPECT_EQ(
      detailed_info.bitmap_container_size_per_genome_section.bitmap_container_size_statistic
         .number_of_values_stored_in_run_containers,
      2354
   );
   EXPECT_EQ(
      detailed_info.bitmap_container_size_per_genome_section.bitmap_container_size_statistic
         .total_bitmap_size_bitset_containers,
      0
   );

   EXPECT_EQ(
      detailed_info.bitmap_container_size_per_genome_section.total_bitmap_size_computed, 96160327
   );
   EXPECT_EQ(
      detailed_info.bitmap_container_size_per_genome_section.total_bitmap_size_frozen, 48185073
   );
   EXPECT_EQ(
      detailed_info.bitmap_container_size_per_genome_section.bitmap_container_size_statistic
         .total_bitmap_size_array_containers,
      119154
   );

   EXPECT_EQ(simple_info.total_size, 60054981);
   EXPECT_EQ(simple_info.sequence_count, 100);
   EXPECT_EQ(simple_info.n_bitmaps_size, 3898);
}

TEST(DatabaseTest, shouldSaveAndReloadDatabaseWithoutErrors) {
   auto first_database = buildTestDatabase();

   const std::string directory = "output/test_serialized_state/";
   if (std::filesystem::exists(directory)) {
      std::filesystem::remove_all(directory);
   }
   std::filesystem::create_directories(directory);

   first_database.saveDatabaseState(directory);

   auto database = silo::Database::loadDatabaseState(directory);

   const auto simple_database_info = database.getDatabaseInfo();

   EXPECT_GT(simple_database_info.total_size, 0);
   EXPECT_EQ(simple_database_info.sequence_count, 100);
}
