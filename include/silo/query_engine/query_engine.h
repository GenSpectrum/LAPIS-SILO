#pragma once

#include <string>

#include "silo/database.h"

namespace silo::query_engine {

struct QueryResult;

class QueryEngine {
  private:
   const silo::Database& database;

  public:
   explicit QueryEngine(const silo::Database& database);

   virtual QueryResult executeQuery(const std::string& query) const;
};

QueryResult executeQuery(const Database& database, const std::string& query);

}  // namespace silo::query_engine
