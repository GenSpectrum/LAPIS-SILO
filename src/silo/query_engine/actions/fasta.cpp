#include "silo/query_engine/actions/fasta.h"

#if defined(__linux__)
#include <malloc.h>
#endif

#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <boost/numeric/conversion/cast.hpp>
#include <duckdb.hpp>
#include <memory>
#include <nlohmann/json.hpp>

#include "silo/common/numbers.h"
#include "silo/common/panic.h"
#include "silo/common/range.h"
#include "silo/database.h"
#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/query_result.h"
#include "silo/zstd/zstd_table_reader.h"

using silo::common::add1;
using silo::common::Range;

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
            std::ranges::find(sequence_names, field.name) != std::end(sequence_names),
         fmt::format(
            "OrderByField {} is not contained in the result of this operation. "
            "The only fields returned by the Fasta action are {} and {}",
            field.name,
            fmt::join(sequence_names, ","),
            primary_key_field
         )
      )
   }
}

namespace {

/// Build SQL statement to retrieve all the entries as rows.
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
      select_clause += fmt::format(", t{0}.sequence as t{0}_sequence", idx);
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
   std::vector<QueryResultEntry>& results,
   duckdb::Connection& connection,
   const std::string& result_table_name,
   const std::vector<std::string>& sequence_names,
   const DatabasePartition& database_partition,
   const std::string& primary_key_column,
   size_t num_result_rows
) {
   for (size_t sequence_idx = 0; sequence_idx < sequence_names.size(); sequence_idx++) {
      const std::string& sequence_name = sequence_names.at(sequence_idx);
      const std::string_view compression_dict =
         database_partition.unaligned_nuc_sequences.at(sequence_name).compression_dictionary;
      silo::ZstdTableReader table_reader(
         connection,
         result_table_name,
         compression_dict,
         fmt::format("t{}_sequence", sequence_idx),
         "TRUE",
         "ORDER BY key"
      );
      table_reader.loadTable();
      std::optional<std::string> genome_buffer;

      // `start_of_partition_in_result` will always be 0 when
      // streaming (result is cleared for every batch), but not for
      // materialized cases (where result is kept across partitions).
      const size_t start_of_partition_in_result = results.size() - num_result_rows;
      const size_t end_of_partition_in_result = results.size();
      for (size_t idx = start_of_partition_in_result; idx < end_of_partition_in_result; idx++) {
         auto current_key = table_reader.next(genome_buffer);
         if (!current_key.has_value()) {
            throw std::runtime_error(
               "Internal Error. The internal parquet file with compressed unaligned sequences does "
               "not contain all the primary keys, that are matched by this query."
            );
         }
         results.at(idx).fields.emplace(primary_key_column, current_key.value());
         if (genome_buffer.has_value()) {
            results.at(idx).fields.emplace(sequence_name, *genome_buffer);
         } else {
            results.at(idx).fields.emplace(sequence_name, std::nullopt);
         }
      }
   }
}

/// Must only be called with num_result_rows > 0. Returns the last processed row_id.
uint32_t addSequencesToResultsForPartition(
   std::vector<std::string>& sequence_names,
   std::vector<QueryResultEntry>& results,
   const DatabasePartition& database_partition,
   const CopyOnWriteBitmap& bitmap,
   const std::string& primary_key_column,
   size_t num_result_rows
) {
   SILO_ASSERT(num_result_rows > 0);

   duckdb::DuckDB duck_db;
   duckdb::Connection connection(duck_db);

   uint64_t unique_identifier_for_function = unique_identifier++;
   std::string key_table_name = fmt::format("tmp_fasta_key_{}", unique_identifier_for_function);
   std::string result_table_name = fmt::format("tmp_result_{}", unique_identifier_for_function);

   // 1. Collect all primary keys into a DuckDB table
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

   // 2. Fill the DuckDB table `key_table_name` with the primary keys
   uint32_t last_row_id = 0;
   {
      duckdb::Appender appender(connection, key_table_name);
      uint32_t num_row_ids = 0;
      for (const uint32_t row_id : *bitmap) {
         if (num_row_ids++ >= num_result_rows) {
            break;
         }
         last_row_id = row_id;

         auto primary_key = database_partition.columns.getValue(primary_key_column, row_id);
         if (primary_key == std::nullopt) {
            throw std::runtime_error(
               fmt::format("Detected primary_key in column '{}' that is null.", primary_key_column)
            );
         }

         // Add the primary key to the DuckDB table
         {
            appender.BeginRow();
            std::string primary_key_string;
            if (holds_alternative<double>(primary_key.value())) {
               primary_key_string = std::to_string(get<double>(primary_key.value()));
            } else if (holds_alternative<int32_t>(primary_key.value())) {
               primary_key_string = std::to_string(get<int32_t>(primary_key.value()));
            } else {
               SILO_ASSERT(holds_alternative<std::string>(primary_key.value()));
               primary_key_string = get<std::string>(primary_key.value());
            }
            appender.Append(duckdb::Value::BLOB(primary_key_string));
            appender.EndRow();
         }
      }
      appender.Close();
   }

   // 3. Create a super table containing all the XX tables together
   {
      const std::string table_query =
         getTableQuery(sequence_names, database_partition, key_table_name);

      const auto create_table_query =
         fmt::format("CREATE TABLE {} AS ({})", result_table_name, table_query);
      SPDLOG_TRACE(
         "Create table query for unaligned in-memory sequence tables: {}", create_table_query
      );

      query(connection, create_table_query);
   }

   // 4. Fill the decompressed sequences into the `QueryResultEntry` entries in `result`
   addSequencesFromResultTableToJson(
      results,
      connection,
      result_table_name,
      sequence_names,
      database_partition,
      primary_key_column,
      num_result_rows
   );

   return last_row_id;
}

// Note: fasta_aligned.cpp has its own PARTITION_CHUNK_SIZE
const size_t PARTITION_CHUNK_SIZE = 10000;

}  // namespace

QueryResult Fasta::execute(const Database& database, std::vector<CopyOnWriteBitmap> bitmap_filter)
   const {
   for (const std::string& sequence_name : sequence_names) {
      CHECK_SILO_QUERY(
         database.unaligned_nuc_sequences.contains(sequence_name),
         "Database does not contain an unaligned sequence with name: '" + sequence_name + "'"
      )
   }

   size_t current_partition = 0;
   // The unprocessed part of the result row numbers, counting from 0
   // from the start of the current partition:
   std::optional<Range<uint32_t>> remaining_result_row_indices;
   return QueryResult::fromGenerator([sequence_names = sequence_names,
                                      remaining_result_row_indices,
                                      bitmap_filter = std::move(bitmap_filter),
                                      &database,
                                      // For an explanation of the iteration algorithm, see the
                                      // comments in the same location (`execute` method) in
                                      // `fasta_aligned.cpp`.
                                      current_partition](std::vector<QueryResultEntry>& results
                                     ) mutable {
      for (; current_partition < database.partitions.size();
           ++current_partition, remaining_result_row_indices = {}) {
         auto& bitmap = bitmap_filter[current_partition];
         if (!remaining_result_row_indices.has_value()) {
            // (See comments in `execute` in `fasta_aligned.cpp`.)
            remaining_result_row_indices = {
               {0, boost::numeric_cast<uint32_t, uint64_t>(bitmap->cardinality())}
            };
         }

         // (See comments in `execute` in `fasta_aligned.cpp`.)
         const Range<uint32_t> result_row_indices =
            remaining_result_row_indices->take(PARTITION_CHUNK_SIZE);
         if (!result_row_indices.isEmpty()) {
            SPDLOG_TRACE(
               "Fasta::execute: refill QueryResult for partition_index {}/{}, {}",
               current_partition,
               database.partitions.size(),
               result_row_indices.toString()
            );
            const std::string& primary_key_column = database.database_config.schema.primary_key;
            const auto& database_partition = database.partitions[current_partition];

            results.resize(result_row_indices.size());
            const uint32_t last_row_id = addSequencesToResultsForPartition(
               sequence_names,
               results,
               database_partition,
               bitmap,
               primary_key_column,
               result_row_indices.size()
            );
#if defined(__linux__)
            SPDLOG_INFO(
               "Fasta sequences generated for partition. Manually invoking malloc_trim() to give "
               "back memory to OS."
            );
            malloc_trim(0);
#endif

            SILO_ASSERT(results.size() == result_row_indices.size());

            *remaining_result_row_indices =
               remaining_result_row_indices->skip(result_row_indices.size());
            // (See comments in `execute` in `fasta_aligned.cpp`.)
            bitmap->removeRange(0, add1(last_row_id));
            // (See comments in `execute` in `fasta_aligned.cpp`.)
            return;
         }
      }
   });
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
