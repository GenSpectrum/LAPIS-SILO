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

#include "silo/preprocessing/identifier.h"

namespace silo {

class ColumnFunction {
   friend class TableReader;
   silo::preprocessing::Identifier column_name;
   std::function<void(size_t, const duckdb::Vector&, size_t)> function;

  public:
   ColumnFunction(
      preprocessing::Identifier column_name,
      std::function<void(size_t, const duckdb::Vector&, size_t)> function
   );
};

class TableReader {
  private:
   duckdb::Connection& connection;
   preprocessing::Identifier table_name;
   preprocessing::Identifier key_column;
   std::vector<ColumnFunction> column_functions;
   std::string where_clause;
   std::string order_by_clause;
   std::unique_ptr<duckdb::MaterializedQueryResult> query_result;

  public:
   explicit TableReader(
      duckdb::Connection& connection,
      preprocessing::Identifier table_name,
      preprocessing::Identifier key_column,
      std::vector<ColumnFunction> column_functions,
      std::string_view where_clause,
      std::string_view order_by_clause
   );

   size_t read();

  private:
   std::string getTableQuery();

   void loadTable();
};
}  // namespace silo
