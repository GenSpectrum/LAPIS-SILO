#pragma once

#include <string_view>

#include "silo/query_engine/operators/query_node.h"
#include "silo/query_engine/query_plan.h"

namespace silo::query_engine {

class Planner {
  public:
   static operators::QueryNodePtr pushdown(
      operators::QueryNodePtr node,
      const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables
   );

   static operators::QueryNodePtr optimize(operators::QueryNodePtr node);

   static QueryPlan planQuery(
      operators::QueryNodePtr node,
      const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
      const config::QueryOptions& query_options,
      std::string_view request_id
   );

   static QueryPlan planSaneqlQuery(
      std::string_view query_string,
      const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
      const config::QueryOptions& query_options,
      std::string_view request_id
   );
};

}  // namespace silo::query_engine
