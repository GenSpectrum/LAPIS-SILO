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

namespace response {
struct QueryResult;
}

class QueryEngine {
  private:
   const silo::Database& database;

  public:
   explicit QueryEngine(const silo::Database& database);

   virtual response::QueryResult executeQuery(const std::string& query) const;
};

static const double DEFAULT_MINIMAL_PROPORTION = 0.02;
struct MutationProportion {
   char mutation_from;
   unsigned position;
   char mutation_to;
   double proportion;
   unsigned count;
};

response::QueryResult executeQuery(const Database& database, const std::string& query);

std::vector<MutationProportion> executeMutations(
   const silo::Database&,
   std::vector<silo::query_engine::OperatorResult>& partition_filters,
   double proportion_threshold
);

uint64_t executeCount(
   const silo::Database& database,
   std::vector<silo::query_engine::OperatorResult>& partition_filters
);

}  // namespace silo

#endif  // SILO_QUERY_ENGINE_H
