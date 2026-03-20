#pragma once

#include "silo/query_engine/action_query.h"
#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/operators/query_node.h"

namespace silo::query_engine {

class Binder {
  public:
   static operators::QueryNodePtr bindQuery(
      ActionQuery action_query,
      const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables
   );
};

}  // namespace silo::query_engine
