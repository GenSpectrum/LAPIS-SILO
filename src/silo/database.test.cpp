#include "silo/database.h"

#include <filesystem>
#include <fstream>
#include <map>

#include <gtest/gtest.h>

#include "config/source/yaml_file.h"
#include "silo/append/database_inserter.h"
#include "silo/append/ndjson_line_reader.h"
#include "silo/common/phylo_tree.h"
#include "silo/config/preprocessing_config.h"
#include "silo/database_info.h"
#include "silo/initialize/initializer.h"
#include "silo/query_engine/actions/aggregated.h"
#include "silo/query_engine/filter/expressions/true.h"
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

   std::map<std::filesystem::path, silo::common::LineageTreeAndIdMap> lineage_trees;
   for (const auto& filename : config.initialization_files.getLineageDefinitionFilenames()) {
      lineage_trees[filename] =
         silo::common::LineageTreeAndIdMap::fromLineageDefinitionFilePath(filename);
   }

   silo::common::PhyloTree phylo_tree_file;
   auto opt_path = config.initialization_files.getPhyloTreeFilename();
   if (opt_path.has_value()) {
      phylo_tree_file = silo::common::PhyloTree::fromFile(opt_path.value());
   }

   auto database = std::make_shared<silo::Database>();
   database->createTable(
      silo::schema::TableName::getDefault(),
      silo::initialize::Initializer::createSchemaFromConfigFiles(
         std::move(database_config),
         reference_genomes,
         lineage_trees,
         phylo_tree_file,
         /*without_unaligned_sequences=*/false
      )
   );
   std::ifstream input(input_directory / "input.ndjson");
   database->appendData(silo::schema::TableName::getDefault(), input);
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
   EXPECT_EQ(database_info.horizontal_bitmaps_size, 9);

   std::string more_data =
      R"(
{"primaryKey": "key6", "pango_lineage": "XBB", "date": "2021-03-19", "region": "Europe", "country": "Switzerland", "division": "Solothurn", "unsorted_date": "2021-02-10", "age": 54, "qc_value": 0.94, "test_boolean_column": true, "float_value": null, "main": {"sequence": "ACGTACGT", "insertions": []}, "testSecondSequence": {"sequence": "ACGT", "insertions": []}, "unaligned_main": "ACGTACGT", "unaligned_testSecondSequence": "ACGT", "E": {"sequence": "MYSF*", "insertions": ["214:EPE"]}, "M": {"sequence": "XXXX*", "insertions": []}}
{"primaryKey": "key7", "pango_lineage": "B", "date": "2021-03-21", "region": "Europe", "country": "Switzerland", "division": "Basel", "unsorted_date": null, "age": null, "qc_value": 0.94, "test_boolean_column": true, "float_value": null, "main": {"sequence": "AAAAAAAA", "insertions": []}, "testSecondSequence": {"sequence": "ACAT", "insertions": []}, "unaligned_main": "AAAAAAAA", "unaligned_testSecondSequence": "ACAT", "E": {"sequence": "MYSF*", "insertions": ["214:EPE"]}, "M": {"sequence": "XXXX*", "insertions": []}}
)";
   std::stringstream more_data_stream{more_data};

   database.appendData(silo::schema::TableName::getDefault(), more_data_stream);

   const auto database_info_after_append = database.getDatabaseInfo();
   auto data_version_after_append = database.getDataVersionTimestamp();

   EXPECT_EQ(database_info_after_append.sequence_count, 7);
   EXPECT_GT(data_version_after_append, data_version);
}

using silo::Nucleotide;
using silo::query_engine::Query;
using silo::query_engine::actions::Aggregated;
using silo::query_engine::filter::expressions::True;
using silo::schema::ColumnIdentifier;
using silo::schema::ColumnType;
using silo::schema::TableSchema;
using silo::storage::column::ColumnMetadata;
using silo::storage::column::SequenceColumnMetadata;
using silo::storage::column::StringColumnMetadata;

TEST(DatabaseTest, canCreateMultipleTablesAndAddData) {
   silo::Database database;
   ColumnIdentifier primary_key{"key", ColumnType::STRING};
   ColumnIdentifier sequence_column{"sequence", ColumnType::NUCLEOTIDE_SEQUENCE};
   std::vector<Nucleotide::Symbol> reference_sequence{
      Nucleotide::Symbol::A, Nucleotide::Symbol::C, Nucleotide::Symbol::G, Nucleotide::Symbol::T
   };

   std::map<ColumnIdentifier, std::shared_ptr<ColumnMetadata>> column_metadata{
      {primary_key, std::make_shared<StringColumnMetadata>(primary_key.name)},
      {sequence_column,
       std::make_shared<SequenceColumnMetadata<Nucleotide>>(
          sequence_column.name, std::move(reference_sequence)
       )},
   };
   silo::schema::TableName first_table_name{"first"};
   database.createTable(first_table_name, TableSchema(column_metadata, primary_key));

   std::ifstream first_table_data{"testBaseData/example.ndjson"};
   database.appendData(first_table_name, first_table_data);

   Query aggregated_all_query(
      first_table_name,
      std::make_unique<True>(),
      std::make_unique<Aggregated>(std::vector<std::string>{})
   );
   auto query_plan_1 =
      database.createQueryPlan(aggregated_all_query, silo::config::QueryOptions{}, "test_query_1");
   std::stringstream result;
   query_plan_1.executeAndWrite(&result, 100);
   ASSERT_EQ(result.str(), "{\"count\":20}\n");

   silo::schema::TableName second_table_name{"second"};
   database.createTable(second_table_name, TableSchema(column_metadata, primary_key));

   std::stringstream second_table_data;
   second_table_data
      << "{\"key\":\"id_1\",\"sequence\":{\"sequence\":\"AAAA\",\"insertions\":[],\"offset\":0}}";
   database.appendData(second_table_name, second_table_data);

   aggregated_all_query.table_name = second_table_name;
   auto query_plan_2 =
      database.createQueryPlan(aggregated_all_query, silo::config::QueryOptions{}, "test_query_2");
   std::stringstream result_2;
   query_plan_2.executeAndWrite(&result_2, 100);
   ASSERT_EQ(result_2.str(), "{\"count\":1}\n");
}
