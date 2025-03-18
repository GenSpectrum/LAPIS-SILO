#include "silo/database.h"

#include <filesystem>
#include <fstream>
#include <istream>

#include <gtest/gtest.h>

#include "config/source/yaml_file.h"
#include "silo/append/append.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/config/preprocessing_config.h"
#include "silo/database_info.h"
#include "silo/database_inserter.h"
#include "silo/initialize/initializer.h"
#include "silo/query_engine/query_engine.h"
#include "silo/storage/reference_genomes.h"

using silo::config::PreprocessingConfig;

namespace {
std::shared_ptr<silo::Database> buildTestDatabase() {
   const std::filesystem::path input_directory{"./testBaseData/unitTestDummyDataset/"};

   auto config = PreprocessingConfig::withDefaults();
   config.overwriteFrom(
      silo::config::YamlFile::readFile(input_directory / "preprocessing_config.yaml")
         .verify(PreprocessingConfig::getConfigSpecification())
   );
   auto database_config = silo::config::DatabaseConfig::getValidatedConfigFromFile(
      input_directory / "database_config.yaml"
   );

   const auto reference_genomes =
      silo::ReferenceGenomes::readFromFile(config.initialize_config.getReferenceGenomeFilename());

   silo::common::LineageTreeAndIdMap lineage_tree;
   if (config.initialize_config.getLineageDefinitionsFilename().has_value()) {
      lineage_tree = silo::common::LineageTreeAndIdMap::fromLineageDefinitionFilePath(
         config.initialize_config.getLineageDefinitionsFilename().value()
      );
   }

   auto database = std::make_shared<silo::Database>(
      silo::Database{silo::initialize::Initializer::createSchemaFromConfigFiles(
         std::move(database_config), std::move(reference_genomes), std::move(lineage_tree)
      )}
   );

   {
      silo::TableInserter table_inserter(&database->table);
      silo::TablePartitionInserter partition_inserter = table_inserter.openNewPartition();

      std::ifstream input(input_directory / "input.ndjson");

      std::string line;
      size_t count = 0;
      while (std::getline(input, line)) {  // Read file line by line
         if (line.empty())
            continue;  // Skip empty lines

         SPDLOG_INFO("Inserting line {}", count++);

         nlohmann::json json_obj = nlohmann::json::parse(line);

         partition_inserter.insert(json_obj);
      }
   }
   return database;
}
}  // namespace

TEST(DatabaseTest, shouldBuildDatabaseWithoutErrors) {
   auto database{buildTestDatabase()};

   const auto simple_database_info = database->getDatabaseInfo();

   EXPECT_GT(simple_database_info.total_size, 0);
   EXPECT_EQ(simple_database_info.sequence_count, 5);
   EXPECT_EQ(simple_database_info.number_of_partitions, 1);
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST(DatabaseTest, shouldReturnCorrectDatabaseInfo) {
   auto database{buildTestDatabase()};

   const auto simple_info = database->getDatabaseInfo();

   EXPECT_EQ(simple_info.total_size, 996);
   EXPECT_EQ(simple_info.sequence_count, 5);
   EXPECT_EQ(simple_info.n_bitmaps_size, 62);
}

TEST(DatabaseTest, shouldSaveAndReloadDatabaseWithoutErrors) {
   auto first_database = buildTestDatabase();

   const std::filesystem::path directory = "output/test_serialized_state/";
   if (std::filesystem::exists(directory)) {
      std::filesystem::remove_all(directory);
   }
   std::filesystem::create_directories(directory);

   const silo::DataVersion::Timestamp data_version_timestamp =
      first_database->getDataVersionTimestamp();

   first_database->saveDatabaseState(directory);

   silo::SiloDataSource data_source =
      silo::SiloDataSource::checkValidDataSource(directory / data_version_timestamp.value).value();

   auto database = silo::Database::loadDatabaseState(data_source);

   const auto simple_database_info = database.getDatabaseInfo();

   EXPECT_GT(simple_database_info.total_size, 0);
   EXPECT_EQ(simple_database_info.sequence_count, 5);
   EXPECT_GT(simple_database_info.n_bitmaps_size, 0);
   EXPECT_EQ(simple_database_info.number_of_partitions, 1);
}
