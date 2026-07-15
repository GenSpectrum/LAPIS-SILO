#include "silo/database.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <map>

#include <fmt/format.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "config/source/yaml_file.h"
#include "silo/common/phylo_tree.h"
#include "silo/config/preprocessing_config.h"
#include "silo/database_info.h"
#include "silo/initialize/initializer.h"
#include "silo/query_engine/illegal_query_exception.h"
#include "silo/query_engine/planner.h"
#include "silo/storage/reference_genomes.h"
#include "silo/test/query_fixture.test.h"

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
      silo::ReferenceGenomes::readFromFile(config.initialization_files.getReferenceGenomeFilepath()
      );

   std::map<std::filesystem::path, silo::common::LineageTreeAndIdMap> lineage_trees;
   for (const auto& file_path : config.initialization_files.getLineageDefinitionFilepaths()) {
      lineage_trees[file_path] =
         silo::common::LineageTreeAndIdMap::fromLineageDefinitionFilePath(file_path);
   }

   silo::common::PhyloTree phylo_tree_file;
   auto opt_path = config.initialization_files.getPhyloTreeFilepath();
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

   const silo::SiloDataSource data_source =
      silo::SiloDataSource::checkValidDataSource(directory / data_version_timestamp.value);

   auto database = silo::Database::loadDatabaseState(data_source);

   const auto database_info = database.getDatabaseInfo();

   EXPECT_EQ(database_info.sequence_count, 5);
   EXPECT_GT(database_info.vertical_bitmaps_size, 0);
   EXPECT_GT(database_info.horizontal_bitmaps_size, 0);

   // When bumping the serialization version, run `make bump-serialization-version` which sets
   // SILO_KEEP_SERIALIZED_STATE=1 to preserve the produced directory for committing to Git.
   if (std::getenv("SILO_KEEP_SERIALIZED_STATE") == nullptr) {
      std::filesystem::remove_all(data_source.path);
   }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST(DatabaseTest, shouldReturnCorrectDatabaseInfoAfterAppendingNewSequences) {
   // If this load fails, the serialization version likely needs to be increased
   // Run `make bump-serialization-version`
   auto database = silo::Database::loadDatabaseState(
      silo::SiloDirectory{"testBaseData/siloSerializedState"}.getMostRecentDataDirectory().value()
   );

   const auto database_info = database.getDatabaseInfo();
   auto data_version = database.getDataVersionTimestamp();

   EXPECT_EQ(database_info.sequence_count, 5);
   EXPECT_GT(database_info.vertical_bitmaps_size, 0);
   EXPECT_EQ(database_info.horizontal_bitmaps_size, 9);

   const std::string more_data =
      R"(
{"primaryKey": "key6", "pango_lineage": "XBB", "date": "2021-03-19", "region": "Europe", "country": "Switzerland", "division": "Solothurn", "unsorted_date": "2021-02-10", "age": 54, "qc_value": 0.94, "test_boolean_column": true, "float_value": null, "main": {"sequence": "ACGTACGT", "insertions": []}, "testSecondSequence": {"sequence": "ACGT", "insertions": []}, "unaligned_main": "ACGTACGT", "unaligned_testSecondSequence": "ACGT", "E": {"sequence": "MYSF*", "insertions": ["4:EPE"]}, "M": {"sequence": "XXXX*", "insertions": []}}
{"primaryKey": "key7", "pango_lineage": "B", "date": "2021-03-21", "region": "Europe", "country": "Switzerland", "division": "Basel", "unsorted_date": null, "age": null, "qc_value": 0.94, "test_boolean_column": true, "float_value": null, "main": {"sequence": "AAAAAAAA", "insertions": []}, "testSecondSequence": {"sequence": "ACAT", "insertions": []}, "unaligned_main": "AAAAAAAA", "unaligned_testSecondSequence": "ACAT", "E": {"sequence": "MYSF*", "insertions": ["4:EPE"]}, "M": {"sequence": "XXXX*", "insertions": []}}
)";
   std::stringstream more_data_stream{more_data};

   database.appendData(silo::schema::TableName::getDefault(), more_data_stream);

   const auto database_info_after_append = database.getDatabaseInfo();
   auto data_version_after_append = database.getDataVersionTimestamp();

   EXPECT_EQ(database_info_after_append.sequence_count, 7);
   EXPECT_GT(data_version_after_append, data_version);
}

namespace {
// Counts the rows of the default table matching `filter` by running a SaneQL count aggregation.
int64_t countWhere(silo::Database& database, const std::string& filter) {
   auto query_plan = silo::query_engine::Planner::planSaneqlQuery(
      fmt::format("default.filter({}).groupBy({{count:=count()}})", filter),
      database.tables,
      silo::config::QueryOptions{},
      "count_query"
   );
   auto result = silo::test::executeQueryToJsonArray(query_plan);
   if (result.empty()) {
      return 0;
   }
   return result.at(0).at("count").get<int64_t>();
}
}  // namespace

TEST(DatabaseTest, updateColumnAssignsScalarValueToMatchingRows) {
   auto database = buildTestDatabase();
   const std::string table = silo::schema::TableName::getDefault().getName();

   // Two rows (key1, key4) start with age 4; key3 has a null age.
   ASSERT_EQ(countWhere(*database, "age = 4"), 2);

   // Assign a scalar int to only the matching rows.
   database->updateColumn(table, "age", "100", "age = 4");
   ASSERT_EQ(countWhere(*database, "age = 4"), 0);
   ASSERT_EQ(countWhere(*database, "age = 100"), 2);

   // A previously-null value can be set to a concrete value.
   ASSERT_EQ(countWhere(*database, "age = 7"), 0);
   database->updateColumn(table, "age", "7", "primaryKey = 'key3'");
   ASSERT_EQ(countWhere(*database, "age = 7"), 1);

   // A SaneQL `null` literal clears the matched rows back to null.
   database->updateColumn(table, "age", "null", "primaryKey = 'key3'");
   ASSERT_EQ(countWhere(*database, "age = 7"), 0);
   ASSERT_EQ(countWhere(*database, "age = null"), 1);

   // Bool values are parsed as the boolean literals 'true'/'false'.
   database->updateColumn(table, "test_boolean_column", "false", "true");
   ASSERT_EQ(countWhere(*database, "test_boolean_column = false"), 5);

   // Date values are SaneQL date literals.
   database->updateColumn(table, "date", "'2000-01-01'::date", "true");
   ASSERT_EQ(countWhere(*database, "date = '2000-01-01'::date"), 5);
}

TEST(DatabaseTest, updateColumnRejectsInvalidRequests) {
   auto database = buildTestDatabase();
   const std::string table = silo::schema::TableName::getDefault().getName();

   // A literal that does not match the column's type is a query error.
   EXPECT_THAT(
      [&]() { database->updateColumn(table, "age", "'not_a_number'", "true"); },
      ThrowsMessage<silo::query_engine::IllegalQueryException>(
         ::testing::HasSubstr("expected integer literal")
      )
   );

   // String columns cannot be updated by this scalar path.
   EXPECT_THAT(
      [&]() { database->updateColumn(table, "division", "Basel", "true"); },
      ThrowsMessage<silo::query_engine::IllegalQueryException>(::testing::HasSubstr("not supported")
      )
   );

   // Unknown columns and tables are reported.
   EXPECT_THAT(
      [&]() { database->updateColumn(table, "does_not_exist", "1", "true"); },
      ThrowsMessage<silo::query_engine::IllegalQueryException>(
         ::testing::HasSubstr("does not contain a column")
      )
   );
}

using silo::Nucleotide;
using silo::schema::ColumnIdentifier;
using silo::schema::ColumnType;
using silo::schema::TableSchema;
using silo::storage::column::ColumnMetadata;
using silo::storage::column::SequenceColumnMetadata;
using silo::storage::column::StringColumnMetadata;

TEST(DatabaseTest, canCreateMultipleTablesAndAddData) {
   silo::Database database;
   ColumnIdentifier primary_key{.name = "key", .type = ColumnType::STRING};
   ColumnIdentifier sequence_column{.name = "sequence", .type = ColumnType::NUCLEOTIDE_SEQUENCE};
   std::vector<Nucleotide::Symbol> reference_sequence{
      Nucleotide::Symbol::A, Nucleotide::Symbol::C, Nucleotide::Symbol::G, Nucleotide::Symbol::T
   };

   const std::map<ColumnIdentifier, std::shared_ptr<ColumnMetadata>> column_metadata{
      {primary_key, std::make_shared<StringColumnMetadata>(primary_key.name)},
      {sequence_column,
       std::make_shared<SequenceColumnMetadata<Nucleotide>>(
          sequence_column.name, std::move(reference_sequence)
       )},
   };
   const silo::schema::TableName first_table_name{"first"};
   database.createTable(
      first_table_name, std::make_shared<TableSchema>(column_metadata, primary_key)
   );
   const silo::schema::TableName second_table_name{"second"};
   database.createTable(
      second_table_name, std::make_shared<TableSchema>(column_metadata, primary_key)
   );

   std::ifstream first_table_data{"testBaseData/example.ndjson"};
   database.appendData(first_table_name, first_table_data);

   auto query_plan_1 = silo::query_engine::Planner::planSaneqlQuery(
      "first.groupBy({count:=count()})",
      database.tables,
      silo::config::QueryOptions{},
      "test_query_1"
   );
   ASSERT_EQ(
      silo::test::executeQueryToJsonArray(query_plan_1), nlohmann::json::array({{{"count", 20}}})
   );

   std::stringstream second_table_data;
   second_table_data
      << R"({"key":"id_1","sequence":{"sequence":"AAAA","insertions":[],"offset":0}})";
   database.appendData(second_table_name, second_table_data);

   auto query_plan_2 = silo::query_engine::Planner::planSaneqlQuery(
      "second.groupBy({count:=count()})",
      database.tables,
      silo::config::QueryOptions{},
      "test_query_2"
   );
   ASSERT_EQ(
      silo::test::executeQueryToJsonArray(query_plan_2), nlohmann::json::array({{{"count", 1}}})
   );
}
