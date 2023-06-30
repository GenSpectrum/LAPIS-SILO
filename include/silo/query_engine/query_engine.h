#ifndef SILO_QUERY_ENGINE_H
#define SILO_QUERY_ENGINE_H

#include <iostream>
#include <memory>
#include <roaring/roaring.hh>
#include <string>
#include <vector>

#include "silo/common/nucleotide_symbols.h"
#include "silo/query_engine/operators/operator.h"

namespace silo {
class Database;
class DatabasePartition;
}  // namespace silo

namespace silo::query_engine {

struct MutationProportion;
struct AggregationResult;
struct QueryResult;

class QueryEngine {
  private:
   const silo::Database& database;

  public:
   explicit QueryEngine(const silo::Database& database);

   virtual QueryResult executeQuery(const std::string& query) const;
};

static const double DEFAULT_MINIMAL_PROPORTION = 0.02;

QueryResult executeQuery(const Database& database, const std::string& query);

std::vector<MutationProportion> executeMutations(
   const Database&,
   std::vector<silo::query_engine::OperatorResult>& partition_filters,
   double proportion_threshold
);

std::vector<AggregationResult> executeCount(
   const Database& database,
   std::vector<silo::query_engine::OperatorResult>& partition_filters
);

}  // namespace silo::query_engine

#endif  // SILO_QUERY_ENGINE_H
