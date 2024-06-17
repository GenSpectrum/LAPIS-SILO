#pragma once

#include <filesystem>
#include <fstream>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <duckdb.hpp>

namespace silo {

class ColumnFunction {
   friend class TableReader;
   std::string column_name;
   std::function<void(size_t, const duckdb::Value&)> function;

  public:
   ColumnFunction(
      std::string column_name,
      std::function<void(size_t, const duckdb::Value&)> function
   );
};

class TableReader {
  private:
   duckdb::Connection& connection;
   std::string table_name;
   std::string key_column;
   std::vector<ColumnFunction> column_functions;
   std::string where_clause;
   std::string order_by_clause;
   std::unique_ptr<duckdb::MaterializedQueryResult> query_result;
   std::unique_ptr<duckdb::DataChunk> current_chunk;
   size_t current_row;
   size_t current_row_in_chunk;

  public:
   explicit TableReader(
      duckdb::Connection& connection,
      std::string_view table_name,
      std::string_view key_column,
      std::vector<ColumnFunction> column_functions,
      std::string_view where_clause,
      std::string_view order_by_clause
   );

   size_t read();

  private:
   std::optional<std::string> nextKey();

   std::string getTableQuery();

   void advanceRow();

   void loadTable();
};
}  // namespace silo
