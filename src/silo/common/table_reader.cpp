#include "silo/common/table_reader.h"

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <duckdb.hpp>

#include "silo/preprocessing/preprocessing_exception.h"

silo::ColumnFunction::ColumnFunction(
   std::string column_name,
   std::function<void(size_t, const duckdb::Value&)> function
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

std::optional<std::string> silo::TableReader::nextKey() {
   if (!current_chunk) {
      return std::nullopt;
   }

   return current_chunk->GetValue(0, current_row_in_chunk).GetValue<std::string>();
}

size_t silo::TableReader::read() {
   loadTable();
   assert(query_result->ColumnCount() == column_functions.size() + 1);
   while (nextKey()) {
      for (size_t column_idx = 0; column_idx < column_functions.size(); column_idx++) {
         column_functions.at(column_idx)
            .function(current_row, current_chunk->GetValue(column_idx + 1, current_row_in_chunk));
      }
      advanceRow();
   }
   return current_row;
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
   current_chunk = query_result->Fetch();
   current_row = 0;
   current_row_in_chunk = 0;

   while (current_chunk && current_chunk->size() == 0) {
      current_chunk = query_result->Fetch();
   }
}

void silo::TableReader::advanceRow() {
   current_row++;
   current_row_in_chunk++;
   if (current_row_in_chunk == current_chunk->size()) {
      current_row_in_chunk = 0;
      current_chunk = query_result->Fetch();
      while (current_chunk && current_chunk->size() == 0) {
         current_chunk = query_result->Fetch();
      }
   }
}
