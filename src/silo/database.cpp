#include "silo/database.h"

#include <array>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <map>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include <arrow/api.h>
#include <arrow/io/memory.h>
#include <arrow/ipc/writer.h>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include "silo/append/database_inserter.h"
#include "silo/common/aa_symbols.h"
#include "silo/common/data_version.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/common/silo_directory.h"
#include "silo/common/version.h"
#include "silo/database_info.h"
#include "silo/persistence/exception.h"
#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/actions/fasta_aligned.h"
#include "silo/query_engine/actions/mutations.h"
#include "silo/query_engine/exec_node/arrow_ipc_sink.h"
#include "silo/query_engine/exec_node/ndjson_sink.h"
#include "silo/query_engine/filter/expressions/true.h"
#include "silo/query_engine/illegal_query_exception.h"
#include "silo/query_engine/query.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/column/sequence_column.h"

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
}  // namespace

namespace silo {

Database::Database(schema::DatabaseSchema database_schema)
    : schema(std::move(database_schema)) {
   for (const auto& [table_name, table_schema] : schema.tables) {
      tables.emplace(table_name, std::make_shared<storage::Table>(table_schema));
   }
}

void Database::createTable(schema::TableName table_name, silo::schema::TableSchema table_schema) {
   tables.emplace(table_name, std::make_shared<storage::Table>(table_schema));
   schema.tables.emplace(std::move(table_name), std::move(table_schema));
}

void Database::appendData(const schema::TableName& table_name, std::istream& input_stream) {
   silo::append::NdjsonLineReader input_data{input_stream};
   SILO_ASSERT(tables.contains(table_name));
   auto& table = tables.at(table_name);
   silo::append::appendDataToTable(table, input_data);
   updateDataVersion();
   SPDLOG_INFO("Database info: {}", getDatabaseInfo());
}

void Database::createNucleotideSequenceTable(
   const std::string& table_name,
   const std::string& primary_key_name,
   const std::string& sequence_name,
   const std::string& reference_sequence,
   const std::vector<std::string>& extra_string_columns
) {
   silo::schema::TableSchema table_schema;
   const schema::ColumnIdentifier primary_key = {
      .name = primary_key_name, .type = schema::ColumnType::STRING
   };
   table_schema.column_metadata.emplace(
      primary_key, std::make_shared<storage::column::StringColumnMetadata>(primary_key_name)
   );
   auto reference_sequence_vector = stringToSymbolVector<Nucleotide>(reference_sequence).value();
   table_schema.column_metadata.emplace(
      schema::ColumnIdentifier{
         .name = sequence_name, .type = schema::ColumnType::NUCLEOTIDE_SEQUENCE
      },
      std::make_shared<storage::column::SequenceColumnMetadata<Nucleotide>>(
         sequence_name, std::move(reference_sequence_vector)
      )
   );
   for (const auto& column_name : extra_string_columns) {
      table_schema.column_metadata.emplace(
         schema::ColumnIdentifier{.name = column_name, .type = schema::ColumnType::STRING},
         std::make_shared<storage::column::StringColumnMetadata>(column_name)
      );
   }
   table_schema.primary_key = primary_key;
   createTable(schema::TableName(table_name), std::move(table_schema));
}

void Database::createGeneTable(
   const std::string& table_name,
   const std::string& primary_key_name,
   const std::string& sequence_name,
   const std::string& reference_sequence,
   const std::vector<std::string>& extra_string_columns
) {
   silo::schema::TableSchema table_schema;
   const schema::ColumnIdentifier primary_key = {
      .name = primary_key_name, .type = schema::ColumnType::STRING
   };
   table_schema.column_metadata.emplace(
      primary_key, std::make_shared<storage::column::StringColumnMetadata>(primary_key_name)
   );
   auto reference_sequence_vector = stringToSymbolVector<AminoAcid>(reference_sequence).value();
   table_schema.column_metadata.emplace(
      schema::ColumnIdentifier{
         .name = sequence_name, .type = schema::ColumnType::AMINO_ACID_SEQUENCE
      },
      std::make_shared<storage::column::SequenceColumnMetadata<AminoAcid>>(
         sequence_name, std::move(reference_sequence_vector)
      )
   );
   for (const auto& column_name : extra_string_columns) {
      table_schema.column_metadata.emplace(
         schema::ColumnIdentifier{.name = column_name, .type = schema::ColumnType::STRING},
         std::make_shared<storage::column::StringColumnMetadata>(column_name)
      );
   }
   table_schema.primary_key = primary_key;
   createTable(schema::TableName(table_name), std::move(table_schema));
}

void Database::appendDataFromFile(const std::string& table_name, const std::string& file_name) {
   std::ifstream input_stream(file_name);
   silo::append::NdjsonLineReader input_data{input_stream};
   silo::append::appendDataToTable(tables.at(schema::TableName{table_name}), input_data);
   SPDLOG_INFO("Database info: {}", getDatabaseInfo());
}

void Database::appendDataFromString(const std::string& table_name, std::string json_string) {
   std::stringstream input_stream(std::move(json_string));
   silo::append::NdjsonLineReader input_data{input_stream};
   silo::append::appendDataToTable(tables.at(schema::TableName{table_name}), input_data);
}

using silo::query_engine::Query;
using silo::query_engine::actions::Action;
using silo::query_engine::actions::FastaAligned;
using silo::query_engine::filter::expressions::Expression;
using silo::query_engine::filter::expressions::True;

void Database::printAllData(const std::string& table_name) const {
   if (!schema.tables.contains(schema::TableName{table_name})) {
      throw std::runtime_error{fmt::format("The database does not contain table {}", table_name)};
   }

   std::vector<std::string> sequence_column_identifiers;
   std::vector<std::string> non_sequence_column_identifiers;
   for (const auto& [column_identifier, _] :
        schema.tables.at(schema::TableName{table_name}).column_metadata) {
      if (isSequenceColumn(column_identifier.type)) {
         if (column_identifier.type != schema::ColumnType::ZSTD_COMPRESSED_STRING) {
            sequence_column_identifiers.push_back(column_identifier.name);
         }
      } else {
         non_sequence_column_identifiers.push_back(column_identifier.name);
      }
   }

   std::unique_ptr<Expression> filter = std::make_unique<True>();
   std::unique_ptr<Action> action = std::make_unique<FastaAligned>(
      std::move(sequence_column_identifiers), std::move(non_sequence_column_identifiers)
   );

   auto query = Query(schema::TableName{table_name}, std::move(filter), std::move(action));
   auto query_plan = createQueryPlan(query, config::QueryOptions{}, "printAllData");
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
      table_schema.getColumnMetadata<storage::column::SequenceColumnPartition<Nucleotide>>(
         sequence_name
      );
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
      table_schema.getColumnMetadata<storage::column::SequenceColumnPartition<AminoAcid>>(
         sequence_name
      );
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
   const nlohmann::json filter_json = nlohmann::json::parse(filter);
   std::unique_ptr<Expression> filter_expression = filter_json;
   auto maybe_table = tables.find(schema::TableName{table_name});
   if (maybe_table == tables.end()) {
      SPDLOG_ERROR("The database does not contain the table {}", table_name);
      return {};
   }
   auto table = maybe_table->second;
   if (table->getNumberOfPartitions() == 0) {
      SPDLOG_WARN("The table is empty");
      return {};
   }
   if (table->getNumberOfPartitions() > 1) {
      SPDLOG_ERROR(
         "The table should not contain more than one partition (actual: {}), internal error.",
         table->getNumberOfPartitions()
      );
      return {};
   }
   auto rewritten_filter_expression =
      filter_expression->rewrite(*table, *table->getPartition(0), Expression::AmbiguityMode::NONE);
   auto filter_operator = rewritten_filter_expression->compile(*table, *table->getPartition(0));
   roaring::Roaring bitmap = filter_operator->evaluate().getConstReference();
   return bitmap;
}

template <typename SymbolType>
std::vector<std::pair<uint64_t, std::string>> Database::getPrevalentMutations(
   const std::string& table_name,
   const std::string& sequence_name,
   double prevalence_threshold,
   const std::string& filter
) const {
   using SymbolMutations = silo::query_engine::actions::Mutations<SymbolType>;

   const nlohmann::json filter_json = nlohmann::json::parse(filter);
   std::unique_ptr<Expression> filter_expression = filter_json;
   std::unique_ptr<Action> action = std::make_unique<SymbolMutations>(
      std::vector<std::string>{sequence_name},
      prevalence_threshold,
      std::vector<std::string_view>{
         SymbolMutations::COUNT_FIELD_NAME, SymbolMutations::MUTATION_FIELD_NAME
      }
   );

   auto query =
      Query(schema::TableName{table_name}, std::move(filter_expression), std::move(action));
   auto query_plan = createQueryPlan(query, config::QueryOptions{}, "getPrevalentMutations");
   std::stringstream result_stream;
   query_engine::exec_node::NdjsonSink output_sink{&result_stream, query_plan.results_schema};
   query_plan.executeAndWrite(output_sink, /*timeout_in_seconds=*/100);

   std::vector<std::pair<uint64_t, std::string>> result;
   std::string json_line;
   while (result_stream >> json_line) {
      auto line = nlohmann::json::parse(json_line);
      SILO_ASSERT(line.contains(SymbolMutations::COUNT_FIELD_NAME));
      const uint64_t count = line[SymbolMutations::COUNT_FIELD_NAME].template get<uint64_t>();
      SILO_ASSERT(line.contains(SymbolMutations::MUTATION_FIELD_NAME));
      const std::string mutation =
         line[SymbolMutations::MUTATION_FIELD_NAME].template get<std::string>();
      result.emplace_back(count, mutation);
   }
   return result;
}

std::vector<std::pair<uint64_t, std::string>> Database::getPrevalentNucMutations(
   const std::string& table_name,
   const std::string& sequence_name,
   double prevalence_threshold,
   const std::string& filter
) const {
   return getPrevalentMutations<Nucleotide>(
      table_name, sequence_name, prevalence_threshold, filter
   );
}

std::vector<std::pair<uint64_t, std::string>> Database::getPrevalentAminoAcidMutations(
   const std::string& table_name,
   const std::string& sequence_name,
   double prevalence_threshold,
   const std::string& filter
) const {
   return getPrevalentMutations<AminoAcid>(table_name, sequence_name, prevalence_threshold, filter);
}

namespace {

void addTableStatisticsToDatabaseInfo(DatabaseInfo& database_info, const storage::Table& table) {
   for (size_t partition_idx = 0; partition_idx < table.getNumberOfPartitions(); ++partition_idx) {
      auto table_partition = table.getPartition(partition_idx);
      // TODO(#743) try to analyze size accuracy relative to RSS
      for (const auto& [_, seq_column] : table_partition->columns.nuc_columns) {
         auto info = seq_column.getInfo();
         database_info.vertical_bitmaps_size += info.vertical_bitmaps_size;
         database_info.horizontal_bitmaps_size += info.horizontal_bitmaps_size;
      }
      for (const auto& [_, seq_column] : table_partition->columns.aa_columns) {
         auto info = seq_column.getInfo();
         database_info.vertical_bitmaps_size += info.vertical_bitmaps_size;
         database_info.horizontal_bitmaps_size += info.horizontal_bitmaps_size;
      }
      database_info.sequence_count += table_partition->sequence_count;
   }
   database_info.number_of_partitions += table.getNumberOfPartitions();
}

}  // namespace

DatabaseInfo Database::getDatabaseInfo() const {
   DatabaseInfo database_info{
      .version = silo::RELEASE_VERSION,
      .sequence_count = 0,
      .vertical_bitmaps_size = 0,
      .horizontal_bitmaps_size = 0,
      .number_of_partitions = 0
   };
   for (const auto& [_, table] : tables) {
      addTableStatisticsToDatabaseInfo(database_info, *table);
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
      std::filesystem::create_directory(versioned_save_directory / table_name.getName());
      table->saveData(versioned_save_directory / table_name.getName());
   }

   data_version_.saveToFile(versioned_save_directory / DATA_VERSION_FILENAME);
}

namespace {
DataVersion loadDataVersion(const std::filesystem::path& filename) {
   if (!std::filesystem::is_regular_file(filename)) {
      auto error = fmt::format("Input file {} could not be opened.", filename.string());
      throw persistence::LoadDatabaseException(error);
   }
   auto data_version = DataVersion::fromFile(filename);
   if (data_version == std::nullopt) {
      auto error_message = fmt::format(
         "Data version file {} did not contain a valid data version", filename.string()
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
   const SiloDirectory silo_directory{save_directory};
   auto silo_data_source = silo_directory.getMostRecentDataDirectory();
   if (silo_data_source.has_value()) {
      return loadDatabaseState(silo_data_source.value());
   }
   return std::nullopt;
}

Database Database::loadDatabaseState(const silo::SiloDataSource& silo_data_source) {
   SPDLOG_INFO("Loading database from data source: {}", silo_data_source.toDebugString());
   const auto save_directory = silo_data_source.path;

   const auto database_schema_path = save_directory / DATABASE_SCHEMA_FILENAME;
   auto schema = schema::DatabaseSchema::loadFromFile(database_schema_path);

   Database database{schema};

   for (const auto& [table_name, _] : schema.tables) {
      SPDLOG_DEBUG("Loading data for table {}", table_name.getName());
      database.tables.at(table_name)->loadData(save_directory / table_name.getName());
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

using query_engine::CopyOnWriteBitmap;
using query_engine::Expression;
using query_engine::Query;
using query_engine::QueryPlan;

[[nodiscard]] QueryPlan Database::createQueryPlan(
   const Query& query,
   const config::QueryOptions& query_options,
   std::string_view request_id
) const {
   SPDLOG_DEBUG("Request Id [{}] - Parsed filter: {}", request_id, query.filter->toString());

   auto maybe_table = tables.find(query.table_name);
   CHECK_SILO_QUERY(
      maybe_table != tables.end(), "The table with name {} is not contained in the database."
   );
   const auto& table = maybe_table->second;

   std::vector<CopyOnWriteBitmap> partition_filters;
   partition_filters.reserve(table->getNumberOfPartitions());
   for (size_t partition_index = 0; partition_index < table->getNumberOfPartitions();
        partition_index++) {
      auto filter_after_rewrite = query.filter->rewrite(
         *table, *table->getPartition(partition_index), Expression::AmbiguityMode::NONE
      );
      SPDLOG_DEBUG(
         "Request Id [{}] - Filter after rewrite for partition {}: {}",
         request_id,
         partition_index,
         filter_after_rewrite->toString()
      );
      auto filter_operator =
         filter_after_rewrite->compile(*table, *table->getPartition(partition_index));
      SPDLOG_DEBUG(
         "Request Id [{}] - Filter operator tree for partition {}: {}",
         request_id,
         partition_index,
         filter_operator->toString()
      );
      partition_filters.emplace_back(filter_operator->evaluate());
   };

   return query.action->toQueryPlan(table, partition_filters, query_options, request_id);
}

std::string Database::executeQueryAsArrowIpc(
   const std::string& table_name,
   const std::string& query_json
) const {
   auto query = Query::parseQuery(query_json);
   query->table_name = schema::TableName{table_name};
   auto query_plan = createQueryPlan(*query, config::QueryOptions{}, "executeQueryAsArrowIpc");
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

   // Build string array with table names
   arrow::StringBuilder builder;
   for (const auto& [table_name, _] : tables) {
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

}  // namespace silo
