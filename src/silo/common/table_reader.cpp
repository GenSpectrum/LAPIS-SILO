#include "silo/common/table_reader.h"

#include <cstddef>
#include <numeric>
#include <string>
#include <string_view>
#include <vector>

#include <fmt/format.h>
#include <oneapi/tbb/parallel_invoke.h>
#include <spdlog/spdlog.h>
#include <duckdb.hpp>

#include "silo/preprocessing/preprocessing_exception.h"

silo::ColumnFunction::ColumnFunction(
   std::string column_name,
   std::function<void(size_t, const duckdb::Vector&, size_t)> function
)
    : column_name(std::move(column_name)),
      function(std::move(function)) {}

silo::TableReader::TableReader(
   duckdb::Connection& connection,
   std::string_view table_name,
   std::string_view key_column,
   std::vector<ColumnFunction> column_functions,
   std::string_view where_clause,
   std::string_view order_by_clause
)
    : connection(connection),
      table_name(table_name),
      key_column(key_column),
      column_functions(std::move(column_functions)),
      where_clause(where_clause),
      order_by_clause(order_by_clause) {}

size_t silo::TableReader::read() {
   loadTable();
   assert(query_result->ColumnCount() == column_functions.size() + 1);
   size_t current_start_of_chunk = 0;
   std::unique_ptr<duckdb::DataChunk> current_chunk = query_result->Fetch();
   while (current_chunk) {
      size_t current_chunk_size = current_chunk->size();

      std::unique_ptr<duckdb::DataChunk> next_chunk;

      tbb::parallel_invoke(
         [&]() {
            for (size_t column_idx = 0; column_idx < column_functions.size(); column_idx++) {
               const auto& column_vector = current_chunk->data[column_idx + 1];
               column_functions.at(column_idx)
                  .function(current_start_of_chunk, column_vector, current_chunk_size);
            }
         },
         [&]() { next_chunk = query_result->Fetch(); }
      );

      current_start_of_chunk += current_chunk_size;
      current_chunk = std::move(next_chunk);
   }

   return current_start_of_chunk;
}

std::string silo::TableReader::getTableQuery() {
   std::string columns;
   for (const auto& column_function : column_functions) {
      columns += fmt::format(", \"{}\"", column_function.column_name);
   }
   return fmt::format(
      "SELECT \"{}\" {} FROM {} WHERE {} {}",
      key_column,
      columns,
      table_name,
      where_clause,
      order_by_clause
   );
}

void silo::TableReader::loadTable() {
   try {
      query_result = connection.Query(getTableQuery());
   } catch (const std::exception& e) {
      SPDLOG_ERROR("Error when executing SQL {}", e.what());
      throw silo::preprocessing::PreprocessingException(fmt::format(
         "SQL for loading the results that the TableReader reads:{}\n"
         "Resulting Error:\n{}",
         getTableQuery(),
         std::string(e.what())
      ));
   }
   if (query_result->HasError()) {
      SPDLOG_ERROR("Error when executing SQL " + query_result->GetError());
      throw silo::preprocessing::PreprocessingException(
         "Error when executing SQL " + query_result->GetError()
      );
   }
}
