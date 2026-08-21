#include "rhydb/database.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <map>
#include <optional>

#include <fmt/format.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "config/source/yaml_file.h"
#include "rhydb/common/lineage_tree.h"
#include "rhydb/common/phylo_tree.h"
#include "rhydb/config/preprocessing_config.h"
#include "rhydb/database_info.h"
#include "rhydb/initialize/initializer.h"
#include "rhydb/query_engine/illegal_query_exception.h"
#include "rhydb/query_engine/planner.h"
#include "rhydb/schema/duplicate_primary_key_exception.h"
#include "rhydb/storage/column/zstd_compressed_string_column.h"
#include "rhydb/storage/reference_genomes.h"
#include "rhydb/test/query_fixture.test.h"

using rhydb::config::PreprocessingConfig;

namespace {
std::shared_ptr<rhydb::Database> buildTestDatabase() {
   const std::filesystem::path input_directory{"./testBaseData/unitTestDummyDataset/"};

   auto config = PreprocessingConfig::withDefaults();
   config.overwriteFrom(
      rhydb::config::YamlFile::readFile(input_directory / "preprocessing_config.yaml")
         .verify(PreprocessingConfig::getConfigSpecification())
   );
   auto database_config = rhydb::config::DatabaseConfig::getValidatedConfigFromFile(
      input_directory / "database_config.yaml"
   );

   auto database = std::make_shared<rhydb::Database>();
   rhydb::initialize::Initializer::loadReferences(
      rhydb::schema::TableName::getDefault(),
      rhydb::ReferenceGenomes::readFromFile(config.initialization_files.getReferenceGenomeFilepath()
      ),
      /*without_unaligned_sequences=*/false,
      *database
   );

   std::map<std::filesystem::path, rhydb::common::LineageTreeAndIdMap> lineage_trees;
   for (const auto& file_path : config.initialization_files.getLineageDefinitionFilepaths()) {
      lineage_trees[file_path] =
         rhydb::common::LineageTreeAndIdMap::fromLineageDefinitionFilePath(file_path);
   }

   rhydb::common::PhyloTree phylo_tree_file;
   auto opt_path = config.initialization_files.getPhyloTreeFilepath();
   if (opt_path.has_value()) {
      phylo_tree_file = rhydb::common::PhyloTree::fromFile(opt_path.value());
   }

   database->createTable(
      rhydb::schema::TableName::getDefault(),
      rhydb::initialize::Initializer::createSchemaFromConfigFiles(
         database_config,
         database->getColumnReferences(rhydb::schema::TableName::getDefault().getName()),
         lineage_trees,
         phylo_tree_file
      )
   );
   std::ifstream input(input_directory / "input.ndjson");
   database->appendData(rhydb::schema::TableName::getDefault(), input);
   return database;
}
}  // namespace

TEST(DatabaseTest, shouldSaveAndReloadDatabaseWithoutErrors) {
   auto first_database = buildTestDatabase();

   const std::filesystem::path directory = "testBaseData/siloSerializedState";

   const rhydb::DataVersion::Timestamp data_version_timestamp =
      first_database->getDataVersionTimestamp();

   first_database->saveDatabaseState(directory);

   const rhydb::RhyDBDataSource data_source =
      rhydb::RhyDBDataSource::checkValidDataSource(directory / data_version_timestamp.value);

   auto database = rhydb::Database::loadDatabaseState(data_source);

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
   auto database = rhydb::Database::loadDatabaseState(
      rhydb::RhyDBDirectory{"testBaseData/siloSerializedState"}.getMostRecentDataDirectory().value()
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

   database.appendData(rhydb::schema::TableName::getDefault(), more_data_stream);

   const auto database_info_after_append = database.getDatabaseInfo();
   auto data_version_after_append = database.getDataVersionTimestamp();

   EXPECT_EQ(database_info_after_append.sequence_count, 7);
   EXPECT_GT(data_version_after_append, data_version);
}

namespace {
// Counts the rows of the default table matching `filter` by running a SaneQL count aggregation.
int64_t countWhere(rhydb::Database& database, const std::string& filter) {
   auto query_plan = rhydb::query_engine::Planner::planSaneqlQuery(
      fmt::format("default.filter({}).groupBy({{count:=count()}})", filter),
      database.tables,
      rhydb::config::QueryOptions{},
      "count_query"
   );
   auto result = rhydb::test::executeQueryToJsonArray(query_plan);
   if (result.empty()) {
      return 0;
   }
   return result.at(0).at("count").get<int64_t>();
}
}  // namespace

TEST(DatabaseTest, updateColumnAssignsScalarValueToMatchingRows) {
   auto database = buildTestDatabase();
   const std::string table = rhydb::schema::TableName::getDefault().getName();

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

   // Indexed string columns can be reassigned; the inverted index stays consistent. `value` is a
   // SaneQL string literal, so it is quoted.
   ASSERT_EQ(countWhere(*database, "division = 'Bern'"), 2);
   database->updateColumn(table, "division", "'Zurich'", "division = 'Bern'");
   ASSERT_EQ(countWhere(*database, "division = 'Bern'"), 0);
   ASSERT_EQ(countWhere(*database, "division = 'Zurich'"), 2);

   // A value not previously present in the dictionary is interned on update.
   database->updateColumn(table, "division", "'Geneva'", "primaryKey = 'key1'");
   ASSERT_EQ(countWhere(*database, "division = 'Geneva'"), 1);

   // A SaneQL `null` literal clears an indexed string back to null, and a concrete value can be set
   // again afterwards.
   database->updateColumn(table, "division", "null", "primaryKey = 'key1'");
   ASSERT_EQ(countWhere(*database, "division = null"), 1);
   database->updateColumn(table, "division", "'Basel'", "primaryKey = 'key1'");
   ASSERT_EQ(countWhere(*database, "division = null"), 0);
   ASSERT_EQ(countWhere(*database, "division = 'Basel'"), 1);
}

TEST(DatabaseTest, updateColumnRejectsInvalidRequests) {
   auto database = buildTestDatabase();
   const std::string table = rhydb::schema::TableName::getDefault().getName();

   // A literal that does not match the column's type is a query error.
   EXPECT_THAT(
      [&]() { database->updateColumn(table, "age", "'not_a_number'", "true"); },
      ThrowsMessage<rhydb::query_engine::IllegalQueryException>(
         ::testing::HasSubstr("expected integer literal")
      )
   );

   // A string literal must be quoted; an int literal is not a valid string value.
   EXPECT_THAT(
      [&]() { database->updateColumn(table, "division", "5", "true"); },
      ThrowsMessage<rhydb::query_engine::IllegalQueryException>(
         ::testing::HasSubstr("expected string literal")
      )
   );

   // A phylogenetic-tree-backed column (primaryKey) cannot be updated.
   EXPECT_THAT(
      [&]() { database->updateColumn(table, "primaryKey", "'new_key'", "true"); },
      ThrowsMessage<rhydb::query_engine::IllegalQueryException>(
         ::testing::HasSubstr("phylogenetic tree")
      )
   );

   // A lineage-indexed column (pango_lineage) cannot be updated.
   EXPECT_THAT(
      [&]() { database->updateColumn(table, "pango_lineage", "'B.1'", "true"); },
      ThrowsMessage<rhydb::query_engine::IllegalQueryException>(::testing::HasSubstr("lineage index"
      ))
   );

   // Unknown columns and tables are reported.
   EXPECT_THAT(
      [&]() { database->updateColumn(table, "does_not_exist", "1", "true"); },
      ThrowsMessage<rhydb::query_engine::IllegalQueryException>(
         ::testing::HasSubstr("does not contain a column")
      )
   );
}

using rhydb::Nucleotide;
using rhydb::schema::ColumnIdentifier;
using rhydb::schema::ColumnType;
using rhydb::schema::TableSchema;
using rhydb::storage::column::ColumnMetadata;
using rhydb::storage::column::SequenceColumnMetadata;
using rhydb::storage::column::StringColumnMetadata;

TEST(DatabaseTest, canCreateMultipleTablesAndAddData) {
   rhydb::Database database;
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
   const rhydb::schema::TableName first_table_name{"first"};
   database.createTable(
      first_table_name, std::make_shared<TableSchema>(column_metadata, primary_key)
   );
   const rhydb::schema::TableName second_table_name{"second"};
   database.createTable(
      second_table_name, std::make_shared<TableSchema>(column_metadata, primary_key)
   );

   std::ifstream first_table_data{"testBaseData/example.ndjson"};
   database.appendData(first_table_name, first_table_data);

   auto query_plan_1 = rhydb::query_engine::Planner::planSaneqlQuery(
      "first.groupBy({count:=count()})",
      database.tables,
      rhydb::config::QueryOptions{},
      "test_query_1"
   );
   ASSERT_EQ(
      rhydb::test::executeQueryToJsonArray(query_plan_1), nlohmann::json::array({{{"count", 20}}})
   );

   std::stringstream second_table_data;
   second_table_data
      << R"({"key":"id_1","sequence":{"sequence":"AAAA","insertions":[],"offset":0}})";
   database.appendData(second_table_name, second_table_data);

   auto query_plan_2 = rhydb::query_engine::Planner::planSaneqlQuery(
      "second.groupBy({count:=count()})",
      database.tables,
      rhydb::config::QueryOptions{},
      "test_query_2"
   );
   ASSERT_EQ(
      rhydb::test::executeQueryToJsonArray(query_plan_2), nlohmann::json::array({{{"count", 1}}})
   );
}

namespace {
// Populates the built-in `reference_genomes` table that createTableFromColumns reads sequence
// references from. Each entry maps a sequence column name to its reference string. The generic path
// takes the column's type from its ColumnDefinition rather than from the table, so the `type`
// column is left empty here (see rhydb::ReferenceEntry).
void populateReferences(
   rhydb::Database& database,
   const std::vector<std::pair<std::string, std::string>>& entries
) {
   std::stringstream data;
   for (const auto& [name, reference] : entries) {
      data << nlohmann::json{{"name", name}, {"reference", reference}, {"type", ""}}.dump() << "\n";
   }
   database.appendData(
      rhydb::schema::TableName{std::string{rhydb::Database::REFERENCE_GENOMES_TABLE_NAME}}, data
   );
}

// Declares in `reference_columns` which reference backs a column, by appending the row directly the
// way a caller of `createTableFromColumns` has to. Each entry is (column name, column type,
// reference name).
void declareColumnReferences(
   rhydb::Database& database,
   const std::string& table_name,
   const std::vector<std::tuple<std::string, std::string, std::string>>& entries
) {
   std::stringstream data;
   for (const auto& [column_name, column_type, reference_name] : entries) {
      data
         << nlohmann::
               json{{"id", fmt::format("{}.{}", table_name, column_name)}, {"table_name", table_name}, {"column_name", column_name}, {"column_type", column_type}, {"reference_name", reference_name}}
                  .dump()
         << "\n";
   }
   database.appendData(
      rhydb::schema::TableName{std::string{rhydb::Database::REFERENCE_COLUMNS_TABLE_NAME}}, data
   );
}
}  // namespace

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST(DatabaseTest, createTableFromColumnsSupportsAllColumnTypes) {
   rhydb::Database database;
   populateReferences(database, {{"seq", "ACGT"}, {"gene", "MFV"}});
   declareColumnReferences(
      database,
      "generic",
      {{"seq", "nucleotide_sequence", "seq"}, {"gene", "amino_acid_sequence", "gene"}}
   );
   database.createTableFromColumns(
      "generic",
      {{.name = "key", .type = "string"},
       {.name = "age", .type = "int"},
       {.name = "qc", .type = "float"},
       {.name = "collected", .type = "date"},
       {.name = "passed", .type = "bool"},
       {.name = "country", .type = "string"},
       {.name = "lineage", .type = "indexed_string"},
       {.name = "seq", .type = "nucleotide_sequence"},
       {.name = "gene", .type = "amino_acid_sequence"}}
   );

   std::stringstream data;
   data << nlohmann::json{
              {"key", "id_1"},
              {"age", 42},
              {"qc", 0.5},
              {"collected", "2021-03-15"},
              {"passed", true},
              {"country", "Switzerland"},
              {"lineage", "B.1"},
              {"seq", {{"sequence", "ACGT"}, {"insertions", nlohmann::json::array()}}},
              {"gene", {{"sequence", "MFV"}, {"insertions", nlohmann::json::array()}}}
           }.dump()
        << "\n";
   database.appendData(rhydb::schema::TableName{"generic"}, data);

   auto query_plan = rhydb::query_engine::Planner::planSaneqlQuery(
      "generic.filter(age = 42 && passed = true && country = 'Switzerland')."
      "project({key, age, qc, country, lineage})",
      database.tables,
      rhydb::config::QueryOptions{},
      "generic_query"
   );
   ASSERT_EQ(
      rhydb::test::executeQueryToJsonArray(query_plan),
      nlohmann::json::array(
         {{{"key", "id_1"}, {"age", 42}, {"qc", 0.5}, {"country", "Switzerland"}, {"lineage", "B.1"}
         }}
      )
   );
}

TEST(DatabaseTest, addReferencesRejectsANameRepeatedWithinOneBatch) {
   rhydb::Database database;
   EXPECT_THROW(
      database.addReferences(
         {{.name = "main", .reference = "ACGT", .type = "nucleotide_sequence"},
          {.name = "main", .reference = "TTTTTTTT", .type = "nucleotide_sequence"}}
      ),
      rhydb::schema::DuplicatePrimaryKeyException
   );
}

TEST(DatabaseTest, addReferencesRejectsANameAlreadyInTheTable) {
   rhydb::Database database;
   database.addReferences({{.name = "main", .reference = "ACGT", .type = "nucleotide_sequence"}});
   EXPECT_THROW(
      database.addReferences({{.name = "main", .reference = "TTTTTTTT", .type = ""}}),
      rhydb::schema::DuplicatePrimaryKeyException
   );
}

TEST(DatabaseTest, addReferencesLeavesTheTableUntouchedWhenABatchIsRejected) {
   // The whole batch is validated up front, so a clash later in the batch must not leave the
   // entries before it appended.
   rhydb::Database database;
   EXPECT_THROW(
      database.addReferences(
         {{.name = "first", .reference = "ACGT", .type = "nucleotide_sequence"},
          {.name = "clash", .reference = "ACGT", .type = "nucleotide_sequence"},
          {.name = "clash", .reference = "TTTT", .type = "nucleotide_sequence"}}
      ),
      rhydb::schema::DuplicatePrimaryKeyException
   );
   EXPECT_TRUE(database.getReferences().empty());
}

TEST(DatabaseTest, addReferencesRejectsANameSharedAcrossTypesBecauseSuchASchemaCannotBeBuilt) {
   // Why the uniqueness check is on the name alone and not on (name, type): the `type` column
   // distinguishes the two entries, but nothing downstream resolves a column by name *and* type.
   // A schema carrying both a nucleotide and an amino acid column called "S" is representable, yet
   // instantiating it throws, because `TableSchema::getColumnMetadata` matches on the name, finds
   // whichever of the two sorts first, and returns nullopt on the type mismatch -- which
   // `storage::Table`'s constructor unwraps. So the pair could never be used, and `addReferences`
   // rejects it up front with a message that names the real problem.
   auto table_schema = std::make_shared<rhydb::schema::TableSchema>();
   const rhydb::schema::ColumnIdentifier key_column{
      .name = "key", .type = rhydb::schema::ColumnType::STRING
   };
   const rhydb::schema::ColumnIdentifier nucleotide_column{
      .name = "S", .type = rhydb::schema::ColumnType::NUCLEOTIDE_SEQUENCE
   };
   const rhydb::schema::ColumnIdentifier amino_acid_column{
      .name = "S", .type = rhydb::schema::ColumnType::AMINO_ACID_SEQUENCE
   };
   table_schema->column_metadata.emplace(
      key_column, std::make_shared<rhydb::storage::column::StringColumnMetadata>(key_column.name)
   );
   table_schema->column_metadata.emplace(
      nucleotide_column,
      std::make_shared<rhydb::storage::column::SequenceColumnMetadata<rhydb::Nucleotide>>(
         nucleotide_column.name, rhydb::ReferenceGenomes::stringToVector<rhydb::Nucleotide>("ACGT")
      )
   );
   table_schema->column_metadata.emplace(
      amino_acid_column,
      std::make_shared<rhydb::storage::column::SequenceColumnMetadata<rhydb::AminoAcid>>(
         amino_acid_column.name, rhydb::ReferenceGenomes::stringToVector<rhydb::AminoAcid>("MFV")
      )
   );
   table_schema->primary_key = key_column;

   // Both columns are in the schema, but a lookup by name can only ever reach one of them.
   ASSERT_EQ(table_schema->getColumnIdentifiers().size(), 3);
   EXPECT_THROW(
      rhydb::storage::Table(rhydb::schema::TableName{"t"}, table_schema), std::bad_optional_access
   );

   // Hence the guard, rather than letting the pair through on the strength of differing types.
   rhydb::Database database;
   EXPECT_THROW(
      database.addReferences(
         {{.name = "S", .reference = "ACGT", .type = "nucleotide_sequence"},
          {.name = "S", .reference = "MFV", .type = "amino_acid_sequence"}}
      ),
      rhydb::schema::DuplicatePrimaryKeyException
   );
}

TEST(DatabaseTest, addReferencesAppendsABatchInOrder) {
   rhydb::Database database;
   database.addReferences(
      {{.name = "main", .reference = "ACGT", .type = "nucleotide_sequence"},
       {.name = "S", .reference = "MFV", .type = "amino_acid_sequence"}}
   );

   auto entries = database.getReferences();
   ASSERT_EQ(entries.size(), 2);
   EXPECT_EQ(entries.at(0).name, "main");
   EXPECT_EQ(entries.at(0).reference, "ACGT");
   EXPECT_EQ(entries.at(0).type, "nucleotide_sequence");
   EXPECT_EQ(entries.at(1).name, "S");
   EXPECT_EQ(entries.at(1).reference, "MFV");
   EXPECT_EQ(entries.at(1).type, "amino_acid_sequence");
}

TEST(DatabaseTest, createTableFromColumnsTakesTheReferenceFromTheDeclaration) {
   rhydb::Database database;
   // The reference is found through the `reference_columns` row, not by matching the column's name
   // against the reference store. Here the two deliberately differ.
   populateReferences(database, {{"a_reference", "ACGTACGT"}});
   declareColumnReferences(database, "sequences", {{"main", "nucleotide_sequence", "a_reference"}});
   database.createTableFromColumns(
      "sequences",
      {{.name = "key", .type = "string"}, {.name = "main", .type = "nucleotide_sequence"}}
   );
   EXPECT_EQ(database.getNucleotideReferenceSequence("sequences", "main"), "ACGTACGT");
}

TEST(DatabaseTest, createTableFromColumnsRequiresADeclarationForASequenceColumn) {
   rhydb::Database database;
   populateReferences(database, {{"main", "ACGTACGT"}});
   // The reference is stored, but nothing says it backs this column.
   EXPECT_THAT(
      [&database]() {
         database.createTableFromColumns(
            "sequences",
            {{.name = "key", .type = "string"}, {.name = "main", .type = "nucleotide_sequence"}}
         );
      },
      ThrowsMessage<std::runtime_error>(::testing::HasSubstr(
         "The column 'main' of table 'sequences' has type 'nucleotide_sequence' and so needs a "
         "reference, but the 'reference_columns' table declares none for it."
      ))
   );
}

TEST(DatabaseTest, createTableFromColumnsRejectsADeclarationThatDisagreesAboutTheColumnType) {
   rhydb::Database database;
   populateReferences(database, {{"main", "ACGTACGT"}});
   declareColumnReferences(database, "sequences", {{"main", "zstd_compressed_string", "main"}});
   EXPECT_THAT(
      [&database]() {
         database.createTableFromColumns(
            "sequences",
            {{.name = "key", .type = "string"}, {.name = "main", .type = "nucleotide_sequence"}}
         );
      },
      ThrowsMessage<std::runtime_error>(
         ::testing::HasSubstr("declares the column 'main' of table 'sequences' as a "
                              "'zstd_compressed_string' column, but "
                              "it is being created with type 'nucleotide_sequence'")
      )
   );
}

TEST(DatabaseTest, getColumnReferencesRejectsAManuallyAppendedRowNamingAnUnknownReference) {
   // Rows can be appended straight to the table, bypassing addColumnReferences' checks, so the
   // mapping is validated when it is read too.
   rhydb::Database database;
   declareColumnReferences(database, "sequences", {{"main", "nucleotide_sequence", "absent"}});
   EXPECT_THAT(
      [&database]() { database.getColumnReferences("sequences"); },
      ThrowsMessage<std::runtime_error>(
         ::testing::HasSubstr("names the reference 'absent', which the 'reference_genomes' table "
                              "does not hold")
      )
   );
}

TEST(DatabaseTest, getColumnReferencesRejectsTwoManuallyAppendedRowsForOneColumn) {
   rhydb::Database database;
   populateReferences(database, {{"first", "ACGT"}, {"second", "TTTT"}});
   // Two rows for one column would otherwise leave which reference wins up to row order. They
   // differ in `id`, so the table's own key does not catch it.
   std::stringstream data;
   data << nlohmann::json{
               {"id", "sequences.main"},
               {"table_name", "sequences"},
               {"column_name", "main"},
               {"column_type", "nucleotide_sequence"},
               {"reference_name", "first"}
   }.dump()
        << "\n"
        << nlohmann::json{
               {"id", "sequences.main.again"},
               {"table_name", "sequences"},
               {"column_name", "main"},
               {"column_type", "nucleotide_sequence"},
               {"reference_name", "second"}
   }.dump()
        << "\n";
   database.appendData(
      rhydb::schema::TableName{std::string{rhydb::Database::REFERENCE_COLUMNS_TABLE_NAME}}, data
   );

   EXPECT_THAT(
      [&database]() { database.getColumnReferences("sequences"); },
      ThrowsMessage<std::runtime_error>(::testing::HasSubstr(
         "declares more than one reference for the column 'main' of table 'sequences'"
      ))
   );
}

TEST(DatabaseTest, createTableFromColumnsAcceptsAReferenceWhoseTypeMatchesTheColumn) {
   rhydb::Database database;
   database.addReferences({{.name = "seq", .reference = "ACGT", .type = "nucleotide_sequence"}});
   declareColumnReferences(database, "samples", {{"seq", "nucleotide_sequence", "seq"}});
   database.createTableFromColumns(
      "samples", {{.name = "key", .type = "string"}, {.name = "seq", .type = "nucleotide_sequence"}}
   );
   EXPECT_EQ(database.getNucleotideReferenceSequence("samples", "seq"), "ACGT");
}

TEST(DatabaseTest, declaringAReferenceOfTheWrongTypeIsRejected) {
   // Without the type check the amino acid column would silently be handed the nucleotide
   // reference "ACGT" and go on to parse it as amino acids.
   rhydb::Database database;
   database.addReferences({{.name = "seq", .reference = "ACGT", .type = "nucleotide_sequence"}});
   EXPECT_THAT(
      [&database]() {
         declareColumnReferences(database, "samples", {{"seq", "amino_acid_sequence", "seq"}});
         database.getColumnReferences("samples");
      },
      ThrowsMessage<std::runtime_error>(::testing::HasSubstr(
         "The column 'seq' of table 'samples' has type 'amino_acid_sequence' and needs a "
         "'amino_acid_sequence' reference, but 'seq' is a 'nucleotide_sequence' reference."
      ))
   );
}

TEST(DatabaseTest, createTableFromColumnsAcceptsAnUntypedReferenceForAnyColumnType) {
   // `register_reference` may omit the type, since the declaration already states the column's.
   rhydb::Database database;
   database.addReferences({{.name = "seq", .reference = "MFV", .type = ""}});
   declareColumnReferences(database, "samples", {{"seq", "amino_acid_sequence", "seq"}});
   database.createTableFromColumns(
      "samples", {{.name = "key", .type = "string"}, {.name = "seq", .type = "amino_acid_sequence"}}
   );
   EXPECT_EQ(database.getAminoAcidReferenceSequence("samples", "seq"), "MFV");
}

TEST(DatabaseTest, createTableFromColumnsLetsTwoColumnsShareOneReference) {
   // The dedup the mapping buys: one stored reference, two columns naming it.
   rhydb::Database database;
   database.addReferences({{.name = "main", .reference = "ACGTACGT", .type = "nucleotide_sequence"}}
   );
   database.addColumnReferences(
      {{.table_name = "samples",
        .column_name = "main",
        .column_type = "nucleotide_sequence",
        .reference_name = "main"},
       {.table_name = "samples",
        .column_name = "unaligned_main",
        .column_type = "zstd_compressed_string",
        .reference_name = "main"}}
   );
   database.createTableFromColumns(
      "samples",
      {{.name = "key", .type = "string"},
       {.name = "main", .type = "nucleotide_sequence"},
       {.name = "unaligned_main", .type = "zstd_compressed_string"}}
   );

   EXPECT_EQ(database.getReferences().size(), 1);
   EXPECT_EQ(database.getNucleotideReferenceSequence("samples", "main"), "ACGTACGT");

   const auto& table_schema = database.tables.at(rhydb::schema::TableName{"samples"})->schema;
   auto metadata =
      table_schema->getColumnMetadata<rhydb::storage::column::ZstdCompressedStringColumn>(
         "unaligned_main"
      );
   ASSERT_TRUE(metadata.has_value());
   EXPECT_EQ(metadata.value()->dictionary_string, "ACGTACGT");
}

TEST(DatabaseTest, createTableFromColumnsScopesReferencesToTheirOwnTable) {
   rhydb::Database database;
   database.addReferences(
      {{.name = "first_ref", .reference = "ACGT", .type = "nucleotide_sequence"},
       {.name = "second_ref", .reference = "TTTT", .type = "nucleotide_sequence"}}
   );
   database.addColumnReferences(
      {{.table_name = "first",
        .column_name = "seq",
        .column_type = "nucleotide_sequence",
        .reference_name = "first_ref"},
       {.table_name = "second",
        .column_name = "seq",
        .column_type = "nucleotide_sequence",
        .reference_name = "second_ref"}}
   );
   database.createTableFromColumns(
      "first", {{.name = "key", .type = "string"}, {.name = "seq", .type = "nucleotide_sequence"}}
   );
   database.createTableFromColumns(
      "second", {{.name = "key", .type = "string"}, {.name = "seq", .type = "nucleotide_sequence"}}
   );

   // Same column name in both tables, each with its own reference and its own mapping row.
   EXPECT_EQ(database.getNucleotideReferenceSequence("first", "seq"), "ACGT");
   EXPECT_EQ(database.getNucleotideReferenceSequence("second", "seq"), "TTTT");
   EXPECT_EQ(database.getColumnReferences("first").size(), 1);
   EXPECT_EQ(database.getColumnReferences("second").size(), 1);
}

TEST(DatabaseTest, addColumnReferencesRejectsDeclaringOneColumnTwice) {
   rhydb::Database database;
   database.addReferences({{.name = "main", .reference = "ACGT", .type = "nucleotide_sequence"}});
   database.addColumnReferences(
      {{.table_name = "samples",
        .column_name = "seq",
        .column_type = "nucleotide_sequence",
        .reference_name = "main"}}
   );
   EXPECT_THAT(
      [&database]() {
         database.addColumnReferences(
            {{.table_name = "samples",
              .column_name = "seq",
              .column_type = "nucleotide_sequence",
              .reference_name = "main"}}
         );
      },
      ThrowsMessage<std::runtime_error>(::testing::HasSubstr(
         "already declares a reference for the column 'seq' of table 'samples'"
      ))
   );
}

TEST(DatabaseTest, addColumnReferencesRejectsAColumnTypeThatTakesNoReference) {
   rhydb::Database database;
   database.addReferences({{.name = "main", .reference = "ACGT", .type = "nucleotide_sequence"}});
   EXPECT_THAT(
      [&database]() {
         database.addColumnReferences(
            {{.table_name = "samples",
              .column_name = "country",
              .column_type = "string",
              .reference_name = "main"}}
         );
      },
      ThrowsMessage<std::runtime_error>(::testing::HasSubstr("a 'string' column takes no reference")
      )
   );
}

TEST(DatabaseTest, addColumnReferencesAcceptsAnUntypedReferenceForEitherKind) {
   rhydb::Database database;
   database.addReferences({{.name = "untyped", .reference = "ACGT", .type = ""}});
   database.addColumnReferences(
      {{.table_name = "t",
        .column_name = "nuc",
        .column_type = "nucleotide_sequence",
        .reference_name = "untyped"},
       {.table_name = "t",
        .column_name = "zstd",
        .column_type = "zstd_compressed_string",
        .reference_name = "untyped"}}
   );
   EXPECT_EQ(database.getColumnReferences("t").size(), 2);
}

TEST(DatabaseTest, createTableFromColumnsReadsZstdDictionaryFromReferenceGenomes) {
   rhydb::Database database;
   // A zstd-compressed string column takes its compression dictionary from the same built-in
   // `reference_genomes` table the sequence columns read their reference from, keyed on the column
   // name.
   populateReferences(database, {{"unaligned_main", "ACGTACGT"}});
   declareColumnReferences(
      database, "sequences", {{"unaligned_main", "zstd_compressed_string", "unaligned_main"}}
   );
   database.createTableFromColumns(
      "sequences",
      {{.name = "key", .type = "string"},
       {.name = "unaligned_main", .type = "zstd_compressed_string"}}
   );

   const auto& table_schema = database.tables.at(rhydb::schema::TableName{"sequences"})->schema;
   auto metadata =
      table_schema->getColumnMetadata<rhydb::storage::column::ZstdCompressedStringColumn>(
         "unaligned_main"
      );
   ASSERT_TRUE(metadata.has_value());
   EXPECT_EQ(metadata.value()->dictionary_string, "ACGTACGT");

   // The value round-trips through the dictionary-based compressor and decompressor.
   std::stringstream data;
   data << R"({"key":"id_1","unaligned_main":"ACGTACGTACGT"})" << "\n";
   database.appendData(rhydb::schema::TableName{"sequences"}, data);

   auto query_plan = rhydb::query_engine::Planner::planSaneqlQuery(
      "sequences.project({key, unaligned_main})",
      database.tables,
      rhydb::config::QueryOptions{},
      "zstd_query"
   );
   ASSERT_EQ(
      rhydb::test::executeQueryToJsonArray(query_plan),
      nlohmann::json::array({{{"key", "id_1"}, {"unaligned_main", "ACGTACGTACGT"}}})
   );
}

TEST(DatabaseTest, createTableFromColumnsRejectsInvalidRequests) {
   rhydb::Database database;

   // No columns at all is rejected (the first column would be the primary key).
   EXPECT_THROW(database.createTableFromColumns("t", {}), std::runtime_error);

   // The first column becomes the primary key and must be a string.
   EXPECT_THROW(
      database.createTableFromColumns("t", {{.name = "id", .type = "int"}}), std::runtime_error
   );

   EXPECT_THROW(
      database.createTableFromColumns("t", {{.name = "c", .type = "not_a_type"}}),
      std::runtime_error
   );

   // A sequence column with nothing declaring its reference is rejected.
   populateReferences(database, {{"other", "ACGT"}});
   EXPECT_THROW(
      database.createTableFromColumns("t", {{.name = "seq", .type = "nucleotide_sequence"}}),
      std::runtime_error
   );

   // The declaration exists but the reference it names contains invalid symbols for a nucleotide
   // sequence.
   rhydb::Database database_with_bad_reference;
   populateReferences(database_with_bad_reference, {{"seq", "XYZ"}});
   declareColumnReferences(
      database_with_bad_reference, "t", {{"seq", "nucleotide_sequence", "seq"}}
   );
   EXPECT_THROW(
      database_with_bad_reference.createTableFromColumns(
         "t", {{.name = "seq", .type = "nucleotide_sequence"}}
      ),
      std::runtime_error
   );

   // An empty reference is rejected for a zstd-compressed column, whose reference doubles as the
   // compression dictionary (zstd would otherwise accept it and compress without a dictionary).
   rhydb::Database database_with_empty_reference;
   populateReferences(database_with_empty_reference, {{"unaligned", ""}});
   declareColumnReferences(
      database_with_empty_reference, "t", {{"unaligned", "zstd_compressed_string", "unaligned"}}
   );
   try {
      database_with_empty_reference.createTableFromColumns(
         "t",
         {{.name = "key", .type = "string"}, {.name = "unaligned", .type = "zstd_compressed_string"}
         }
      );
      FAIL() << "Expected an empty compression dictionary to be rejected";
   } catch (const std::runtime_error& exception) {
      EXPECT_THAT(
         exception.what(), ::testing::HasSubstr("requires a non-empty compression dictionary")
      );
   }

   // A duplicated column name is rejected.
   EXPECT_THROW(
      database.createTableFromColumns(
         "t", {{.name = "dup", .type = "string"}, {.name = "dup", .type = "int"}}
      ),
      std::runtime_error
   );
}
