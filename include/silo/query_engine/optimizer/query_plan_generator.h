#pragma once

#include "silo/database.h"
#include "silo/query_engine/query.h"
#include "silo/query_engine/query_plan.h"

namespace silo::query_engine::optimizer {

class QueryPlanGenerator {
   std::shared_ptr<silo::Database> database;

  public:
   QueryPlanGenerator(std::shared_ptr<silo::Database> database);

   QueryPlan createQueryPlan(const Query& query, std::ostream& output_stream);
};

}  // namespace silo::query_engine::optimizer
