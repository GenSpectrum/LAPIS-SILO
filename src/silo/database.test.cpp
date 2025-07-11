#include "silo/database.h"

#include <filesystem>
#include <fstream>
#include <istream>

#include <gtest/gtest.h>

#include "config/source/yaml_file.h"
#include "silo/append/append.h"
#include "silo/append/database_inserter.h"
#include "silo/append/ndjson_line_reader.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/common/phylo_tree.h"
#include "silo/config/preprocessing_config.h"
#include "silo/database_info.h"
#include "silo/initialize/initializer.h"
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
      silo::ReferenceGenomes::readFromFile(config.initialization_files.getReferenceGenomeFilename()
      );

   silo::common::LineageTreeAndIdMap lineage_tree;
   if (config.initialization_files.getLineageDefinitionsFilename().has_value()) {
      lineage_tree = silo::common::LineageTreeAndIdMap::fromLineageDefinitionFilePath(
         config.initialization_files.getLineageDefinitionsFilename().value()
      );
   }

   silo::common::PhyloTree phylo_tree_file;
   auto opt_path = config.initialization_files.getPhyloTreeFilename();
   if (opt_path.has_value()) {
      phylo_tree_file = silo::common::PhyloTree::fromFile(opt_path.value());
   }

   auto database = std::make_shared<silo::Database>(
      silo::Database{silo::initialize::Initializer::createSchemaFromConfigFiles(
         std::move(database_config),
         std::move(reference_genomes),
         std::move(lineage_tree),
         std::move(phylo_tree_file)
      )}
   );
   std::ifstream input(input_directory / "input.ndjson");
   silo::append::appendDataToDatabase(*database, silo::append::NdjsonLineReader{input});
   return database;
}
}  // namespace

TEST(DatabaseTest, shouldSaveAndReloadDatabaseWithoutErrors) {
   auto first_database = buildTestDatabase();

   const std::filesystem::path directory = "testBaseData/siloSerializedState";

   const silo::DataVersion::Timestamp data_version_timestamp =
      first_database->getDataVersionTimestamp();

   first_database->saveDatabaseState(directory);

   silo::SiloDataSource data_source =
      silo::SiloDataSource::checkValidDataSource(directory / data_version_timestamp.value);

   auto database = silo::Database::loadDatabaseState(data_source);

   const auto database_info = database.getDatabaseInfo();

   EXPECT_EQ(database_info.sequence_count, 5);
   EXPECT_GT(database_info.vertical_bitmaps_size, 0);
   EXPECT_GT(database_info.horizontal_bitmaps_size, 0);
   EXPECT_EQ(database_info.number_of_partitions, 1);

   // If the serialization version changes, comment out the next line to build a new database for
   // the next test. Then add the produced directory to Git and remove the old serialized state.
   // Also bump CURRENT_SILO_SERIALIZATION_VERSION in src/silo/common/data_version.h
   std::filesystem::remove_all(data_source.path);
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST(DatabaseTest, shouldReturnCorrectDatabaseInfoAfterAppendingNewSequences) {
   // If this load fails, the serialization version likely needs to be increased
   auto database = silo::Database::loadDatabaseState(
      silo::SiloDirectory{"testBaseData/siloSerializedState"}.getMostRecentDataDirectory().value()
   );

   const auto database_info = database.getDatabaseInfo();
   auto data_version = database.getDataVersionTimestamp();

   EXPECT_EQ(database_info.sequence_count, 5);
   EXPECT_GT(database_info.vertical_bitmaps_size, 0);
   EXPECT_EQ(database_info.horizontal_bitmaps_size, 123);

   std::vector<nlohmann::json> more_data{
      nlohmann::json::parse(
         R"({"metadata":{"primaryKey":"key6","pango_lineage":"XBB","date":"2021-03-19","region":"Europe","country":"Switzerland","division":"Solothurn","unsorted_date":"2021-02-10","age":54,"qc_value":0.94,"test_boolean_column":true},"aminoAcidInsertions":{"E":["214:EPE"],"M":[]},"nucleotideInsertions":{"main":[],"testSecondSequence":[]},"alignedAminoAcidSequences":{"E":"MYSF*","M":"XXXX*"},"alignedNucleotideSequences":{"main":"ACGTACGT","testSecondSequence":"ACGT"},"unalignedNucleotideSequences":{"main":"ACGTACGT","testSecondSequence":"ACGT"}})"
      ),
      nlohmann::json::parse(
         R"({"metadata":{"primaryKey":"key7","pango_lineage":"B","date":"2021-03-21","region":"Europe","country":"Switzerland","division":"Basel","unsorted_date":null,"age":null,"qc_value":0.94,"test_boolean_column":true},"aminoAcidInsertions":{"E":["214:EPE"],"M":[]},"nucleotideInsertions":{"main":[],"testSecondSequence":[]},"alignedAminoAcidSequences":{"E":"MYSF*","M":"XXXX*"},"alignedNucleotideSequences":{"main":"AAAAAAAA","testSecondSequence":"ACAT"},"unalignedNucleotideSequences":{"main":"AAAAAAAA","testSecondSequence":"ACAT"}})"
      )
   };

   silo::append::appendDataToDatabase(database, more_data);

   const auto database_info_after_append = database.getDatabaseInfo();
   auto data_version_after_append = database.getDataVersionTimestamp();

   EXPECT_EQ(database_info_after_append.sequence_count, 7);
   EXPECT_GT(data_version_after_append, data_version);
}
