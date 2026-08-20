#include "rhydb/database.h"

#include <filesystem>
#include <fstream>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include <arrow/api.h>
#include <arrow/io/memory.h>
#include <arrow/ipc/writer.h>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include "rhydb/append/table_inserter.h"
#include "rhydb/common/aa_symbols.h"
#include "rhydb/common/data_version.h"
#include "rhydb/common/nucleotide_symbols.h"
#include "rhydb/common/panic.h"
#include "rhydb/common/silo_directory.h"
#include "rhydb/common/version.h"
#include "rhydb/database_info.h"
#include "rhydb/persistence/exception.h"
#include "rhydb/query_engine/exec_node/arrow_ipc_sink.h"
#include "rhydb/query_engine/exec_node/ndjson_sink.h"
#include "rhydb/query_engine/illegal_query_exception.h"
#include "rhydb/query_engine/operators/query_node.h"
#include "rhydb/query_engine/operators/table_scan_node.h"
#include "rhydb/query_engine/planner.h"
#include "rhydb/query_engine/saneql/ast_to_query.h"
#include "rhydb/query_engine/saneql/parser.h"
#include "rhydb/query_engine/scalar_column_update.h"
#include "rhydb/query_engine/scalar_expressions/literal.h"
#include "rhydb/schema/database_schema.h"
#include "rhydb/storage/column/dictionary_encoded_column.h"
#include "rhydb/storage/column/row_id.h"
#include "rhydb/storage/column/sequence_column.h"
#include "rhydb/storage/column/string_column.h"
#include "rhydb/storage/column/zstd_compressed_string_column.h"

namespace {
template <typename SymbolType>
std::optional<std::vector<typename SymbolType::Symbol>> stringToSymbolVector(
   const std::string& sequence
) {
   const size_t size = sequence.size();
   std::vector<typename SymbolType::Symbol> result;
   result.reserve(size);
   for (size_t i = 0; i < size; ++i) {
      if (i + 1 < size && sequence[i] == '\\') {
         ++i;
      }
      auto symbol = SymbolType::charToSymbol(sequence[i]);
      if (symbol == std::nullopt) {
         return std::nullopt;
      }
      result.emplace_back(symbol.value());
   }
   return result;
}

template <typename SymbolType>
std::string symbolVectorToString(const std::vector<typename SymbolType::Symbol>& sequence) {
   const size_t size = sequence.size();
   std::string result;
   result.reserve(size);
   for (const auto& symbol : sequence) {
      auto character = SymbolType::symbolToChar(symbol);
      result += character;
   }
   return result;
}

using rhydb::schema::ColumnType;
ColumnType parseColumnTypeName(const std::string& type_name) {
   static const std::map<std::string, ColumnType> TYPE_NAMES{
      {"string", ColumnType::STRING},
      {"indexed_string", ColumnType::DICTIONARY_ENCODED},
      {"date", ColumnType::DATE32},
      {"bool", ColumnType::BOOL},
      {"int", ColumnType::INT32},
      {"float", ColumnType::FLOAT},
      {"nucleotide_sequence", ColumnType::NUCLEOTIDE_SEQUENCE},
      {"amino_acid_sequence", ColumnType::AMINO_ACID_SEQUENCE},
      {"zstd_compressed_string", ColumnType::ZSTD_COMPRESSED_STRING},
   };
   auto iter = TYPE_NAMES.find(type_name);
   if (iter == TYPE_NAMES.end()) {
      throw std::runtime_error(fmt::format(
         "Unknown column type '{}'. Supported types are: string, indexed_string, date, bool, int, "
         "float, nucleotide_sequence, amino_acid_sequence, zstd_compressed_string.",
         type_name
      ));
   }
   return iter->second;
}

/// The column types whose metadata requires a reference string (looked up in the built-in
/// `reference_genomes` table): the two sequence types and the zstd-compressed unaligned sequence
/// type.
bool columnTypeNeedsReference(ColumnType type) {
   return type == ColumnType::NUCLEOTIDE_SEQUENCE || type == ColumnType::AMINO_ACID_SEQUENCE ||
          type == ColumnType::ZSTD_COMPRESSED_STRING;
}

std::shared_ptr<rhydb::storage::column::ColumnMetadata> makeColumnMetadata(
   const std::string& name,
   ColumnType type,
   const std::string& details
) {
   namespace column = rhydb::storage::column;
   switch (type) {
      case ColumnType::STRING:
         return std::make_shared<column::StringColumnMetadata>(name);
      case ColumnType::DICTIONARY_ENCODED:
         return std::make_shared<column::DictionaryEncodedColumnMetadata>(name);
      case ColumnType::DATE32:
      case ColumnType::BOOL:
      case ColumnType::INT32:
      case ColumnType::FLOAT:
         return std::make_shared<column::ColumnMetadata>(name);
      case ColumnType::NUCLEOTIDE_SEQUENCE: {
         if (details.empty()) {
            throw std::runtime_error(
               fmt::format("Column '{}' requires a non-empty reference sequence", name)
            );
         }
         auto reference = stringToSymbolVector<rhydb::Nucleotide>(details);
         if (!reference.has_value()) {
            throw std::runtime_error(
               fmt::format("Column '{}' has an invalid nucleotide reference sequence", name)
            );
         }
         return std::make_shared<column::SequenceColumnMetadata<rhydb::Nucleotide>>(
            name, std::move(reference.value())
         );
      }
      case ColumnType::AMINO_ACID_SEQUENCE: {
         if (details.empty()) {
            throw std::runtime_error(
               fmt::format("Column '{}' requires a non-empty reference sequence", name)
            );
         }
         auto reference = stringToSymbolVector<rhydb::AminoAcid>(details);
         if (!reference.has_value()) {
            throw std::runtime_error(
               fmt::format("Column '{}' has an invalid amino acid reference sequence", name)
            );
         }
         return std::make_shared<column::SequenceColumnMetadata<rhydb::AminoAcid>>(
            name, std::move(reference.value())
         );
      }
      case ColumnType::ZSTD_COMPRESSED_STRING: {
         if (details.empty()) {
            throw std::runtime_error(
               fmt::format("Column '{}' requires a non-empty compression dictionary", name)
            );
         }
         return std::make_shared<column::ZstdCompressedStringColumnMetadata>(name, details);
      }
      case ColumnType::INT64:
         throw std::runtime_error("INT64 columns cannot be created");
   }
   SILO_UNREACHABLE();
}
}  // namespace

namespace rhydb {

Database::Database() {
   createReferenceGenomesTable();
}

Database::Database(schema::DatabaseSchema database_schema)
    : schema(std::move(database_schema)) {
   for (const auto& [table_name, table_schema] : schema.tables) {
      tables.emplace(table_name, std::make_shared<storage::Table>(table_name, table_schema));
   }
}

void Database::createReferenceGenomesTable() {
   auto table_schema = std::make_shared<schema::TableSchema>();
   const schema::ColumnIdentifier name_column{.name = "name", .type = ColumnType::STRING};
   const schema::ColumnIdentifier reference_column{.name = "reference", .type = ColumnType::STRING};
   const schema::ColumnIdentifier type_column{.name = "type", .type = ColumnType::STRING};
   table_schema->column_metadata.emplace(
      name_column, std::make_shared<storage::column::StringColumnMetadata>(name_column.name)
   );
   table_schema->column_metadata.emplace(
      reference_column,
      std::make_shared<storage::column::StringColumnMetadata>(reference_column.name)
   );
   table_schema->column_metadata.emplace(
      type_column, std::make_shared<storage::column::StringColumnMetadata>(type_column.name)
   );
   table_schema->primary_key = name_column;
   createTable(
      schema::TableName{std::string{REFERENCE_GENOMES_TABLE_NAME}}, std::move(table_schema)
   );
}

void Database::createTable(
   schema::TableName table_name,
   std::shared_ptr<schema::TableSchema> table_schema
) {
   tables.emplace(table_name, std::make_shared<storage::Table>(table_name, table_schema));
   schema.tables.emplace(std::move(table_name), std::move(table_schema));
}

void Database::appendData(
   const schema::TableName& table_name,
   std::istream& input_stream,
   append::ClusteredBufferingOptions clustering_options
) {
   rhydb::append::NdjsonLineReader input_data{input_stream};
   SILO_ASSERT(tables.contains(table_name));
   auto& table = tables.at(table_name);
   rhydb::append::appendDataToTable(table, input_data, std::move(clustering_options));
   updateDataVersion();
   SPDLOG_INFO("Database info: {}", getDatabaseInfo());
}

void Database::createTableFromColumns(
   const std::string& table_name,
   const std::vector<ColumnDefinition>& columns
) {
   const schema::TableName requested_table_name{table_name};
   if (tables.contains(requested_table_name)) {
      throw std::runtime_error(fmt::format("A table named '{}' already exists.", table_name));
   }
   if (columns.empty()) {
      throw std::runtime_error(
         "Cannot create a table without columns: the first column becomes the primary key."
      );
   }
   auto table_schema = std::make_shared<schema::TableSchema>();
   std::optional<schema::ColumnIdentifier> primary_key;
   std::set<std::string> seen_names;
   for (const auto& column : columns) {
      if (!seen_names.insert(column.name).second) {
         throw std::runtime_error(fmt::format("Duplicate column name '{}'.", column.name));
      }
      const auto type = parseColumnTypeName(column.type);
      if (!primary_key.has_value()) {
         // The first column listed becomes the table's primary key, which must be a string (the
         // same constraint the config-driven preprocessing path enforces on its primary key).
         if (type != schema::ColumnType::STRING) {
            throw std::runtime_error(fmt::format(
               "The first column '{}' becomes the primary key and must be of type 'string', but "
               "has "
               "type '{}'.",
               column.name,
               column.type
            ));
         }
         primary_key = schema::ColumnIdentifier{.name = column.name, .type = type};
      }
      std::string reference;
      if (columnTypeNeedsReference(type)) {
         reference = lookupReferenceForColumn(column.name);
      }
      const schema::ColumnIdentifier column_identifier{.name = column.name, .type = type};
      table_schema->column_metadata.emplace(
         column_identifier, makeColumnMetadata(column.name, type, reference)
      );
   }
   table_schema->primary_key = primary_key.value();
   createTable(schema::TableName(table_name), std::move(table_schema));
}

std::string Database::lookupReferenceForColumn(const std::string& column_name) {
   // The `reference_genomes` table is built-in (see `createReferenceGenomesTable`), so it always
   // exists with its `name` and `reference` string columns.
   auto table_iter = tables.find(schema::TableName{std::string{REFERENCE_GENOMES_TABLE_NAME}});
   SILO_ASSERT(table_iter != tables.end());
   const auto& reference_columns = table_iter->second->columns.string_columns;
   auto name_column_iter = reference_columns.find("name");
   auto reference_column_iter = reference_columns.find("reference");
   SILO_ASSERT(
      name_column_iter != reference_columns.end() &&
      reference_column_iter != reference_columns.end()
   );
   const auto& name_column = name_column_iter->second;
   const auto& reference_column = reference_column_iter->second;

   const roaring::Roaring all_rows =
      getFilteredBitmap(std::string{REFERENCE_GENOMES_TABLE_NAME}, "true");
   for (const uint32_t global_row_id : all_rows) {
      const auto row_id = storage::column::RowId::fromGlobal(global_row_id);
      if (name_column.isNull(row_id) || name_column.getValueString(row_id) != column_name) {
         continue;
      }
      if (reference_column.isNull(row_id)) {
         throw std::runtime_error(fmt::format(
            "The '{}' entry named '{}' has a null reference.",
            REFERENCE_GENOMES_TABLE_NAME,
            column_name
         ));
      }
      return reference_column.getValueString(row_id);
   }
   throw std::runtime_error(fmt::format(
      "The '{}' table has no entry named '{}' for the column being created.",
      REFERENCE_GENOMES_TABLE_NAME,
      column_name
   ));
}

std::vector<ReferenceEntry> Database::getReferences() {
   // The `reference_genomes` table is built-in (see `createReferenceGenomesTable`), so it always
   // exists with its `name`, `reference` and `type` string columns.
   auto table_iter = tables.find(schema::TableName{std::string{REFERENCE_GENOMES_TABLE_NAME}});
   SILO_ASSERT(table_iter != tables.end());
   const auto& reference_columns = table_iter->second->columns.string_columns;
   auto name_column_iter = reference_columns.find("name");
   auto reference_column_iter = reference_columns.find("reference");
   auto type_column_iter = reference_columns.find("type");
   SILO_ASSERT(
      name_column_iter != reference_columns.end() &&
      reference_column_iter != reference_columns.end() &&
      type_column_iter != reference_columns.end()
   );
   const auto& name_column = name_column_iter->second;
   const auto& reference_column = reference_column_iter->second;

   std::vector<ReferenceEntry> entries;
   const roaring::Roaring all_rows =
      getFilteredBitmap(std::string{REFERENCE_GENOMES_TABLE_NAME}, "true");
   for (const uint32_t global_row_id : all_rows) {
      const auto row_id = storage::column::RowId::fromGlobal(global_row_id);
      ReferenceEntry entry;
      if (!name_column.isNull(row_id)) {
         entry.name = name_column.getValueString(row_id);
      }
      if (!reference_column.isNull(row_id)) {
         entry.reference = reference_column.getValueString(row_id);
      }
      // The generic `createTableFromColumns` path does not use the `type` column and writes it as
      // an empty string or null; treat a null value as an untyped entry.
      if (!type_column_iter->second.isNull(row_id)) {
         entry.type = type_column_iter->second.getValueString(row_id);
      }
      entries.push_back(std::move(entry));
   }
   return entries;
}

void Database::appendDataFromFile(const std::string& table_name, const std::string& file_path) {
   std::ifstream input_stream(file_path);
   rhydb::append::NdjsonLineReader input_data{input_stream};
   rhydb::append::appendDataToTable(tables.at(schema::TableName{table_name}), input_data);
   SPDLOG_INFO("Database info: {}", getDatabaseInfo());
}

void Database::appendDataFromString(const std::string& table_name, std::string json_string) {
   std::stringstream input_stream(std::move(json_string));
   rhydb::append::NdjsonLineReader input_data{input_stream};
   rhydb::append::appendDataToTable(tables.at(schema::TableName{table_name}), input_data);
}

using rhydb::query_engine::scalar_expressions::BoolLiteral;
using rhydb::query_engine::scalar_expressions::ScalarExpression;

void Database::printAllData(const std::string& table_name) const {
   auto table_iter = tables.find(schema::TableName{table_name});
   if (table_iter == tables.end()) {
      throw std::runtime_error{fmt::format("The database does not contain table {}", table_name)};
   }
   auto table = table_iter->second;

   std::vector<schema::ColumnIdentifier> all_columns = table->schema->getColumnIdentifiers();

   auto query_node = std::make_unique<query_engine::operators::TableScanNode>(
      table, std::make_unique<BoolLiteral>(true), std::move(all_columns)
   );

   auto query_plan = query_engine::Planner::planQuery(
      std::move(query_node), tables, config::QueryOptions{}, "printAllData"
   );
   query_engine::exec_node::NdjsonSink output_sink{&std::cout, query_plan.results_schema};
   query_plan.executeAndWrite(output_sink, /*timeout_in_seconds=*/100);
}

std::string Database::getNucleotideReferenceSequence(
   const std::string& table_name,
   const std::string& sequence_name
) {
   auto maybe_table_schema = schema.tables.find(schema::TableName{table_name});
   if (maybe_table_schema == schema.tables.end()) {
      throw std::runtime_error{fmt::format("The database does not contain table {}", table_name)};
   }
   const auto& table_schema = maybe_table_schema->second;

   auto maybe_sequence_column_metadata =
      table_schema->getColumnMetadata<storage::column::SequenceColumn<Nucleotide>>(sequence_name);
   if (maybe_sequence_column_metadata == std::nullopt) {
      SPDLOG_ERROR(
         "The database table {} does not contain the nucleotide sequence column {}",
         table_name,
         sequence_name
      );
      return {};
   }

   const auto& sequence_column_metadata = maybe_sequence_column_metadata.value();

   return symbolVectorToString<Nucleotide>(sequence_column_metadata->reference_sequence);
}

std::string Database::getAminoAcidReferenceSequence(
   const std::string& table_name,
   const std::string& sequence_name
) {
   auto maybe_table_schema = schema.tables.find(schema::TableName{table_name});
   if (maybe_table_schema == schema.tables.end()) {
      throw std::runtime_error{fmt::format("The database does not contain table {}", table_name)};
   }
   const auto& table_schema = maybe_table_schema->second;

   auto maybe_sequence_column_metadata =
      table_schema->getColumnMetadata<storage::column::SequenceColumn<AminoAcid>>(sequence_name);
   if (maybe_sequence_column_metadata == std::nullopt) {
      SPDLOG_ERROR(
         "The database table {} does not contain the nucleotide sequence column {}",
         table_name,
         sequence_name
      );
      return {};
   }

   const auto& sequence_column_metadata = maybe_sequence_column_metadata.value();

   return symbolVectorToString<AminoAcid>(sequence_column_metadata->reference_sequence);
}

roaring::Roaring Database::getFilteredBitmap(
   const std::string& table_name,
   const std::string& filter
) {
   auto maybe_table = tables.find(schema::TableName{table_name});
   if (maybe_table == tables.end()) {
      SPDLOG_ERROR("The database does not contain the table {}", table_name);
      return {};
   }
   auto table = maybe_table->second;

   query_engine::saneql::Parser parser(filter);
   auto ast = parser.parse();
   auto filter_expression =
      query_engine::saneql::convertToFilter(*ast, table->schema->getColumnIdentifiers());

   auto rewritten_filter_expression =
      filter_expression->rewrite(*table, ScalarExpression::AmbiguityMode::NONE);
   auto filter_operator = rewritten_filter_expression->compile(*table);
   roaring::Roaring bitmap = filter_operator->evaluate().toRoaring();
   return bitmap;
}

// Currently updates are not thread-safe. An update during concurrent access is not allowed
void Database::updateColumn(
   const std::string& table_name,
   const std::string& column_name,
   const std::string& value,
   const std::string& filter_expression
) {
   auto maybe_table = tables.find(schema::TableName{table_name});
   if (maybe_table == tables.end()) {
      throw query_engine::IllegalQueryException(
         fmt::format("The database does not contain the table '{}'", table_name)
      );
   }
   auto& table = *maybe_table->second;

   const auto column = table.schema->getColumn(column_name);
   if (!column.has_value()) {
      throw query_engine::IllegalQueryException(
         fmt::format("The table '{}' does not contain a column '{}'", table_name, column_name)
      );
   }

   const roaring::Roaring row_ids = getFilteredBitmap(table_name, filter_expression);
   query_engine::assignScalarLiteralToColumn(table.columns, *column, value, row_ids);

   // The update mutates persisted table data, so bump the data version like appendData does; this
   // keeps getDataVersionTimestamp() and versioned save directories consistent with the change.
   updateDataVersion();
}

namespace {

void addTableStatisticsToDatabaseInfo(DatabaseInfo& database_info, const storage::Table& table) {
   // TODO(#743) try to analyze size accuracy relative to RSS
   for (const auto& [_, seq_column] : table.columns.nuc_columns) {
      auto info = seq_column.getInfo();
      database_info.vertical_bitmaps_size += info.vertical_bitmaps_size;
      database_info.horizontal_bitmaps_size += info.horizontal_bitmaps_size;
   }
   for (const auto& [_, seq_column] : table.columns.aa_columns) {
      auto info = seq_column.getInfo();
      database_info.vertical_bitmaps_size += info.vertical_bitmaps_size;
      database_info.horizontal_bitmaps_size += info.horizontal_bitmaps_size;
   }
   database_info.sequence_count += table.row_layout.numRows();
}

}  // namespace

DatabaseInfo Database::getDatabaseInfo() const {
   DatabaseInfo database_info{
      .version = rhydb::RELEASE_VERSION,
      .sequence_count = 0,
      .vertical_bitmaps_size = 0,
      .horizontal_bitmaps_size = 0
   };
   const auto default_table = tables.find(schema::TableName::getDefault());
   if (default_table != tables.end()) {
      addTableStatisticsToDatabaseInfo(database_info, *default_table->second);
   }
   return database_info;
}

const std::string DATABASE_SCHEMA_FILENAME = "database_schema.silo";
const std::string DATA_VERSION_FILENAME = "data_version.silo";

void Database::saveDatabaseState(const std::filesystem::path& save_directory) {
   if (getDataVersionTimestamp().value.empty()) {
      throw persistence::SaveDatabaseException(
         "Corrupted database (Data version is empty). Cannot save database."
      );
   }

   std::filesystem::create_directory(save_directory);
   if (!std::filesystem::exists(save_directory)) {
      auto error = fmt::format(
         "Could not create the directory '{}' which contains the saved databases outputs",
         save_directory.string()
      );
      SPDLOG_ERROR(error);
      throw persistence::SaveDatabaseException(error);
   }

   const std::filesystem::path versioned_save_directory =
      save_directory / getDataVersionTimestamp().value;
   SPDLOG_INFO("Saving database to '{}'", versioned_save_directory.string());

   if (std::filesystem::exists(versioned_save_directory)) {
      auto error = fmt::format(
         "In the output directory {} there already exists a file/folder with the name equal to "
         "the current data-version: {}",
         save_directory.string(),
         getDataVersionTimestamp().value
      );
      SPDLOG_ERROR(error);
      throw persistence::SaveDatabaseException(error);
   }

   std::filesystem::create_directory(versioned_save_directory);

   SPDLOG_INFO("Saving database schema");

   const auto database_schema_path = versioned_save_directory / DATABASE_SCHEMA_FILENAME;
   schema.saveToFile(database_schema_path);

   for (const auto& [table_name, table] : tables) {
      SPDLOG_DEBUG("Saving table data for table {}", table_name.getName());
      const std::filesystem::path table_file =
         versioned_save_directory / (table_name.getName() + ".silo");
      table->saveData(table_file);
   }

   data_version_.saveToFile(versioned_save_directory / DATA_VERSION_FILENAME);
}

namespace {
DataVersion loadDataVersion(const std::filesystem::path& file_path) {
   if (!std::filesystem::is_regular_file(file_path)) {
      auto error = fmt::format("Input file {} could not be opened.", file_path.string());
      throw persistence::LoadDatabaseException(error);
   }
   auto data_version = DataVersion::fromFile(file_path);
   if (data_version == std::nullopt) {
      auto error_message = fmt::format(
         "Data version file {} did not contain a valid data version", file_path.string()
      );
      SPDLOG_ERROR(error_message);
      throw persistence::LoadDatabaseException(error_message);
   }
   return data_version.value();
}
}  // namespace

std::optional<Database> Database::loadDatabaseStateFromPath(
   const std::filesystem::path& save_directory
) {
   const RhyDBDirectory silo_directory{save_directory};
   auto silo_data_source = silo_directory.getMostRecentDataDirectory();
   if (silo_data_source.has_value()) {
      return loadDatabaseState(silo_data_source.value());
   }
   return std::nullopt;
}

Database Database::loadDatabaseState(const rhydb::RhyDBDataSource& silo_data_source) {
   SPDLOG_INFO("Loading database from data source: {}", silo_data_source.toDebugString());
   const auto save_directory = silo_data_source.path;

   const auto database_schema_path = save_directory / DATABASE_SCHEMA_FILENAME;
   auto schema = schema::DatabaseSchema::loadFromFile(database_schema_path);

   Database database{schema};

   for (const auto& [table_name, _] : schema.tables) {
      SPDLOG_DEBUG("Loading data for table {}", table_name.getName());
      database.tables.at(table_name)->loadData(save_directory / (table_name.getName() + ".silo"));
   }

   database.data_version_ = loadDataVersion(save_directory / DATA_VERSION_FILENAME);

   SPDLOG_INFO(
      "Finished loading data_version from {}", (save_directory / DATA_VERSION_FILENAME).string()
   );

   SPDLOG_INFO("Database info after loading: {}", database.getDatabaseInfo());

   return database;
}

DataVersion::Timestamp Database::getDataVersionTimestamp() const {
   return data_version_.timestamp;
}

void Database::updateDataVersion() {
   data_version_ = DataVersion::mineDataVersion();
   SPDLOG_DEBUG("Data version was set to {}", data_version_.toString());
}

std::string Database::executeQueryAsArrowIpc(const std::string& query_string) const {
   auto query_plan = query_engine::Planner::planSaneqlQuery(
      query_string, tables, config::QueryOptions{}, "executeQueryAsArrowIpc"
   );

   constexpr uint64_t DEFAULT_TIMEOUT_SECONDS = 120;
   std::ostringstream output_stream;
   auto output_sink =
      query_engine::exec_node::ArrowIpcSink::make(&output_stream, query_plan.results_schema);
   if (!output_sink.status().ok()) {
      throw std::runtime_error(
         fmt::format("Failed to create Arrow IPC writer: {}", output_sink.status().message())
      );
   }
   query_plan.executeAndWrite(output_sink.ValueUnsafe(), DEFAULT_TIMEOUT_SECONDS);
   return output_stream.str();
}

std::string Database::getTablesAsArrowIpc() const {
   std::string result;
   auto status = getTablesAsArrowIpcImpl().Value(&result);
   if (!status.ok()) {
      throw std::runtime_error(
         fmt::format("Failed to write finish ArrowIpcSink: {}", status.message())
      );
   }
   return result;
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
arrow::Result<std::string> Database::getTablesAsArrowIpcImpl() const {
   // Create schema with a single "table_name" column
   auto arrow_schema = arrow::schema({arrow::field("table_name", arrow::utf8())});

   // Build string array with table names, skipping internal tables (those whose name starts with
   // an underscore). The built-in `reference_genomes` table has no underscore and is listed here
   // like any other table.
   arrow::StringBuilder builder;
   for (const auto& [table_name, _] : tables) {
      if (table_name.getName().starts_with('_')) {
         continue;
      }
      ARROW_RETURN_NOT_OK(builder.Append(table_name.getName()));
   }

   ARROW_ASSIGN_OR_RAISE(auto array, builder.Finish());

   ARROW_ASSIGN_OR_RAISE(auto exec_batch, arrow::ExecBatch::Make({array}, array->length()));

   std::ostringstream output_stream;
   ARROW_ASSIGN_OR_RAISE(
      auto output_sink, query_engine::exec_node::ArrowIpcSink::make(&output_stream, arrow_schema)
   );

   ARROW_RETURN_NOT_OK(output_sink.writeBatch(exec_batch));
   ARROW_RETURN_NOT_OK(output_sink.finish());
   return output_stream.str();
}

}  // namespace rhydb
