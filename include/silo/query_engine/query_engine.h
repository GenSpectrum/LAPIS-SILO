#ifndef SILO_QUERY_ENGINE_H
#define SILO_QUERY_ENGINE_H

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <roaring/roaring.hh>

#include "silo/query_engine/operators/operator.h"

namespace silo {
class Database;
class DatabasePartition;
}  // namespace silo

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

#endif  // SILO_QUERY_ENGINE_H
