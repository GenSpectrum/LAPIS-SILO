#include "silo/database.h"

#include <filesystem>

#include <gtest/gtest.h>

#include "silo/common/nucleotide_symbols.h"
#include "silo/config/preprocessing_config.h"
#include "silo/config/util/config_repository.h"
#include "silo/config/util/yaml_file.h"
#include "silo/database_info.h"
#include "silo/preprocessing/preprocessor.h"
#include "silo/preprocessing/sql_function.h"
#include "silo/query_engine/query_engine.h"
#include "silo/storage/reference_genomes.h"

namespace {
silo::Database buildTestDatabase() {
   const std::string input_directory{"./testBaseData/exampleDataset/"};

   silo::config::PreprocessingConfig config;
   config.overwrite(silo::config::YamlFile("./testBaseData/test_preprocessing_config.yaml"));

   const auto database_config =
      silo::config::ConfigRepository().getValidatedConfig(input_directory + "database_config.yaml");

   const auto reference_genomes =
      silo::ReferenceGenomes::readFromFile(config.getReferenceGenomeFilename());

   const auto alias_lookup =
      silo::PangoLineageAliasLookup::readFromFile(config.getPangoLineageDefinitionFilename());

   silo::preprocessing::Preprocessor preprocessor(
      config, database_config, reference_genomes, alias_lookup
   );
   return preprocessor.preprocess();
}
}  // namespace

TEST(DatabaseTest, shouldBuildDatabaseWithoutErrors) {
   auto database{buildTestDatabase()};

   const auto simple_database_info = database.getDatabaseInfo();

   EXPECT_GT(simple_database_info.total_size, 0);
   EXPECT_EQ(simple_database_info.sequence_count, 100);
   EXPECT_EQ(simple_database_info.number_of_partitions, 11);
}

TEST(DatabaseTest, shouldSuccessfullyBuildDatabaseWithoutPartitionBy) {
   const std::string input_directory{"./testBaseData/"};

   silo::config::PreprocessingConfig config;
   config.overwrite(silo::config::YamlFile(input_directory + "test_preprocessing_config.yaml"));

   const auto database_config = silo::config::ConfigRepository().getValidatedConfig(
      input_directory + "test_database_config_without_partition_by.yaml"
   );

   const auto reference_genomes =
      silo::ReferenceGenomes::readFromFile(config.getReferenceGenomeFilename());

   const auto alias_lookup =
      silo::PangoLineageAliasLookup::readFromFile(config.getPangoLineageDefinitionFilename());

   silo::preprocessing::Preprocessor preprocessor(
      config, database_config, reference_genomes, alias_lookup
   );
   auto database = preprocessor.preprocess();

   const auto simple_database_info = database.getDatabaseInfo();

   EXPECT_GT(simple_database_info.total_size, 0);
   EXPECT_EQ(simple_database_info.sequence_count, 100);
   EXPECT_EQ(simple_database_info.number_of_partitions, 1);
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST(DatabaseTest, shouldReturnCorrectDatabaseInfo) {
   auto database{buildTestDatabase()};

   const auto detailed_info = database.detailedDatabaseInfo().sequences.at("main");
   const auto simple_info = database.getDatabaseInfo();

   EXPECT_EQ(
      detailed_info.bitmap_size_per_symbol.size_in_bytes.at(silo::Nucleotide::Symbol::A), 2775910
   );
   EXPECT_EQ(
      detailed_info.bitmap_size_per_symbol.size_in_bytes.at(silo::Nucleotide::Symbol::GAP), 2661831
   );

   EXPECT_EQ(
      detailed_info.bitmap_container_size_per_genome_section.bitmap_container_size_statistic
         .number_of_bitset_containers,
      0
   );
   EXPECT_EQ(
      detailed_info.bitmap_container_size_per_genome_section.bitmap_container_size_statistic
         .number_of_values_stored_in_run_containers,
      2875
   );
   EXPECT_EQ(
      detailed_info.bitmap_container_size_per_genome_section.bitmap_container_size_statistic
         .total_bitmap_size_bitset_containers,
      0
   );

   EXPECT_EQ(
      detailed_info.bitmap_container_size_per_genome_section.total_bitmap_size_computed, 42629964
   );
   EXPECT_EQ(
      detailed_info.bitmap_container_size_per_genome_section.total_bitmap_size_frozen, 21433248
   );
   EXPECT_EQ(
      detailed_info.bitmap_container_size_per_genome_section.bitmap_container_size_statistic
         .total_bitmap_size_array_containers,
      133240
   );

   EXPECT_EQ(simple_info.total_size, 26589464);
   EXPECT_EQ(simple_info.sequence_count, 100);
   EXPECT_EQ(simple_info.n_bitmaps_size, 3898);
}

TEST(DatabaseTest, shouldSaveAndReloadDatabaseWithoutErrors) {
   auto first_database = buildTestDatabase();

   const std::filesystem::path directory = "output/test_serialized_state/";
   if (std::filesystem::exists(directory)) {
      std::filesystem::remove_all(directory);
   }
   std::filesystem::create_directories(directory);

   const silo::DataVersion::Timestamp data_version_timestamp =
      first_database.getDataVersionTimestamp();

   first_database.saveDatabaseState(directory);

   auto database = silo::Database::loadDatabaseState(directory / data_version_timestamp.value);

   const auto simple_database_info = database.getDatabaseInfo();

   EXPECT_GT(simple_database_info.total_size, 0);
   EXPECT_EQ(simple_database_info.sequence_count, 100);
   EXPECT_GT(simple_database_info.n_bitmaps_size, 0);
   EXPECT_EQ(simple_database_info.number_of_partitions, 11);
}
