#include "silo/query_engine/actions/fasta.h"

#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <duckdb.hpp>
#include <nlohmann/json.hpp>

#include "silo/database.h"
#include "silo/query_engine/operator_result.h"
#include "silo/query_engine/query_parse_exception.h"
#include "silo/query_engine/query_result.h"
#include "silo/zstdfasta/zstdfasta_table_reader.h"

namespace silo {
class Database;
}  // namespace silo

namespace {

std::unique_ptr<duckdb::MaterializedQueryResult> query(
   duckdb::Connection& connection,
   std::string sql_query
) {
   SPDLOG_DEBUG("Fasta Action - DuckDB Query:\n{}", sql_query);
   auto result = connection.Query(sql_query);
   SPDLOG_DEBUG("Fasta Action - DuckDB Result:\n{}", result->ToString());
   if (result->HasError()) {
      throw std::runtime_error(result->ToString());
   }
   return result;
}

std::atomic<uint64_t> unique_identifier = 0;
}  // namespace

namespace silo::query_engine::actions {

Fasta::Fasta(std::vector<std::string>&& sequence_names)
    : sequence_names(sequence_names) {}

void Fasta::validateOrderByFields(const Database& database) const {
   const std::string& primary_key_field = database.database_config.schema.primary_key;
   for (const OrderByField& field : order_by_fields) {
      CHECK_SILO_QUERY(
         field.name == primary_key_field ||
            std::find(sequence_names.begin(), sequence_names.end(), field.name) !=
               std::end(sequence_names),
         fmt::format(
            "OrderByField {} is not contained in the result of this operation. "
            "The only fields returned by the Fasta action are {} and {}",
            field.name,
            fmt::join(sequence_names, ","),
            primary_key_field
         )
      );
   }
}

namespace {

std::string getTableQuery(
   const std::vector<std::string>& sequence_names,
   const DatabasePartition& database_partition,
   const std::string& key_table_name
) {
   std::vector<std::filesystem::path> read_file_sqls;
   read_file_sqls.reserve(sequence_names.size());
   for (const auto& sequence_name : sequence_names) {
      read_file_sqls.emplace_back(
         database_partition.unaligned_nuc_sequences.at(sequence_name).getReadSQL()
      );
   }

   std::string select_clause;
   std::string table_clause;
   std::string join_clause;
   for (size_t idx = 0; idx < read_file_sqls.size(); idx++) {
      const auto& sql_to_read_file = read_file_sqls.at(idx);
      select_clause +=
         fmt::format(", t{0}.unaligned_nuc_{1} as t{0}_sequence", idx, sequence_names.at(idx));
      table_clause += fmt::format(", ({}) t{}", sql_to_read_file.string(), idx);
      join_clause += fmt::format(" AND key_table.key = t{}.key", idx);
   }

   std::string duckdb_table_query = fmt::format(
      "SELECT key_table.key {} FROM {} key_table {} WHERE TRUE {}",
      select_clause,
      key_table_name,
      table_clause,
      join_clause
   );
   return duckdb_table_query;
}

void addSequencesFromResultTableToJson(
   QueryResult& results,
   duckdb::Connection& connection,
   const std::string& result_table_name,
   const std::vector<std::string>& sequence_names,
   const DatabasePartition& database_partition,
   size_t number_of_values
) {
   for (size_t sequence_idx = 0; sequence_idx < sequence_names.size(); sequence_idx++) {
      const std::string& sequence_name = sequence_names.at(sequence_idx);
      const std::string_view compression_dict =
         database_partition.unaligned_nuc_sequences.at(sequence_name).compression_dictionary;
      silo::ZstdFastaTableReader table_reader(
         connection,
         result_table_name,
         compression_dict,
         fmt::format("t{}_sequence", sequence_idx),
         "TRUE",
         "ORDER BY key"
      );
      table_reader.loadTable();
      std::optional<std::string> genome_buffer;

      const size_t start_of_partition_in_result = results.query_result.size() - number_of_values;
      const size_t end_of_partition_in_result = results.query_result.size();
      for (size_t idx = start_of_partition_in_result; idx < end_of_partition_in_result; idx++) {
         auto current_key = table_reader.next(genome_buffer);
         assert(current_key.has_value());
         if (genome_buffer.has_value()) {
            results.query_result.at(idx).fields.emplace(sequence_name, *genome_buffer);
         } else {
            results.query_result.at(idx).fields.emplace(sequence_name, std::nullopt);
         }
      }
   }
}

}  // namespace

void Fasta::addSequencesToResultsForPartition(
   QueryResult& results,
   const DatabasePartition& database_partition,
   const OperatorResult& bitmap,
   const std::string& primary_key_column
) const {
   duckdb::DuckDB duck_db;
   duckdb::Connection connection(duck_db);

   const size_t number_of_values = bitmap->cardinality();

   if (bitmap->isEmpty()) {
      SPDLOG_TRACE("Skipping empty partition!");
      return;
   }

   uint64_t unique_identifier_for_function = unique_identifier++;
   std::string key_table_name = fmt::format("tmp_fasta_key_{}", unique_identifier_for_function);
   std::string result_table_name = fmt::format("tmp_result_{}", unique_identifier_for_function);

   query(
      connection,
      fmt::format(
         "CREATE TABLE {} ("
         "    key STRING"
         ");",
         key_table_name
      )
   );
   SPDLOG_TRACE("Created temporary duckdb table for holding keys");

   duckdb::Appender appender(connection, key_table_name);

   for (const uint32_t sequence_id : *bitmap) {
      appender.BeginRow();
      auto primary_key = database_partition.columns.getValue(primary_key_column, sequence_id);
      if (primary_key == std::nullopt) {
         throw std::runtime_error(
            fmt::format("Detected primary_key in column '{}' that is null.", primary_key_column)
         );
      }
      std::string primary_key_string;
      if (holds_alternative<double>(primary_key.value())) {
         primary_key_string = std::to_string(get<double>(primary_key.value()));
      } else if (holds_alternative<int32_t>(primary_key.value())) {
         primary_key_string = std::to_string(get<int32_t>(primary_key.value()));
      } else {
         assert(holds_alternative<std::string>(primary_key.value()));
         primary_key_string = get<std::string>(primary_key.value());
      }
      appender.Append(duckdb::Value::BLOB(primary_key_string));

      // Also add the key to the entries for later
      QueryResultEntry entry;
      entry.fields.emplace(primary_key_column, primary_key.value());
      results.query_result.emplace_back(std::move(entry));

      appender.EndRow();
      appender.Flush();
   }
   appender.Close();

   const std::string table_query =
      getTableQuery(sequence_names, database_partition, key_table_name);

   SPDLOG_TRACE(
      "Create table query for unaligned in-memory sequence tables: {}",
      fmt::format("CREATE TABLE {} AS ({})", result_table_name, table_query)
   );

   query(connection, fmt::format("CREATE TABLE {} AS ({})", result_table_name, table_query));

   addSequencesFromResultTableToJson(
      results, connection, result_table_name, sequence_names, database_partition, number_of_values
   );

   query(connection, fmt::format("DROP TABLE {};", result_table_name));
   query(connection, fmt::format("DROP TABLE {};", key_table_name));
}

QueryResult Fasta::execute(const Database& database, std::vector<OperatorResult> bitmap_filter)
   const {
   for (const std::string& sequence_name : sequence_names) {
      CHECK_SILO_QUERY(
         database.unaligned_nuc_sequences.contains(sequence_name),
         "Database does not contain an unaligned sequence with name: '" + sequence_name + "'"
      );
   }

   const std::string& primary_key_column = database.database_config.schema.primary_key;

   size_t total_count = 0;
   for (auto& filter : bitmap_filter) {
      total_count += filter->cardinality();
   }
   CHECK_SILO_QUERY(
      total_count <= SEQUENCE_LIMIT,
      fmt::format("Fasta action currently limited to {} sequences", SEQUENCE_LIMIT)
   );

   QueryResult results;
   results.query_result.reserve(total_count);

   for (uint32_t partition_index = 0; partition_index < database.partitions.size();
        ++partition_index) {
      const auto& database_partition = database.partitions[partition_index];
      const auto& bitmap = bitmap_filter[partition_index];

      addSequencesToResultsForPartition(results, database_partition, bitmap, primary_key_column);
   }

   return results;
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Fasta>& action) {
   CHECK_SILO_QUERY(
      json.contains("sequenceName") &&
         (json["sequenceName"].is_string() || json["sequenceName"].is_array()),
      "Fasta action must have the field sequenceName of type string or an array of "
      "strings"
   );
   std::vector<std::string> sequence_names;
   if (json["sequenceName"].is_array()) {
      for (const auto& child : json["sequenceName"]) {
         CHECK_SILO_QUERY(
            child.is_string(),
            "Fasta action must have the field sequenceName of type string or an array "
            "of strings; while parsing array encountered the element " +
               child.dump() + " which is not of type string"
         );
         sequence_names.emplace_back(child.get<std::string>());
      }
   } else {
      sequence_names.emplace_back(json["sequenceName"].get<std::string>());
   }
   action = std::make_unique<Fasta>(std::move(sequence_names));
}

}  // namespace silo::query_engine::actions
