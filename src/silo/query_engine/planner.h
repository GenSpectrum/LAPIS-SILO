#pragma once

#include "silo/query_engine/operators/query_node.h"
#include "silo/query_engine/query_plan.h"

namespace silo::query_engine {

class Planner {
  public:
   static operators::QueryNodePtr optimize(operators::QueryNodePtr node);

   static QueryPlan planQuery(
      const operators::QueryNodePtr node,
      const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
      const config::QueryOptions& query_options,
      std::string_view request_id
   );
};

}  // namespace silo::query_engine
